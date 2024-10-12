// Copyright 2023 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/mman.h>
#include <sys/poll.h>

#include <vector>

#include "base/bits.h"
#include "base/containers/span.h"
#include "base/files/file_util.h"
#include "base/json/json_writer.h"
#include "base/logging.h"
#include "base/numerics/safe_conversions.h"
#include "base/rand_util.h"
#include "base/run_loop.h"
#include "base/test/bind.h"
#include "components/viz/common/resources/shared_image_format.h"
#include "gpu/command_buffer/common/mailbox.h"
#include "gpu/command_buffer/common/shared_image_usage.h"
#include "gpu/command_buffer/service/shared_context_state.h"
#include "gpu/command_buffer/service/shared_image/shared_image_backing.h"
#include "gpu/command_buffer/service/shared_image/shared_image_factory.h"
#include "gpu/command_buffer/service/shared_image/shared_image_manager.h"
#include "gpu/command_buffer/service/shared_image/shared_image_representation.h"
#include "gpu/config/gpu_feature_info.h"
#include "media/gpu/chromeos/chromeos_compressed_gpu_memory_buffer_video_frame_utils.h"
#include "media/gpu/chromeos/image_processor_factory.h"
#include "media/gpu/chromeos/platform_video_frame_utils.h"
#include "media/gpu/chromeos/vulkan_image_processor.h"
#include "media/gpu/test/image.h"
#include "media/gpu/test/video_test_environment.h"
#include "media/gpu/video_frame_mapper_factory.h"
#include "testing/perf/perf_result_reporter.h"
#include "third_party/libyuv/include/libyuv.h"
#include "ui/gl/gl_context.h"
#include "ui/gl/gl_surface.h"
#include "ui/gl/gl_utils.h"
#include "ui/gl/init/gl_factory.h"
#include "ui/gl/scoped_make_current.h"
#include "ui/gl/test/gl_surface_test_support.h"

#if BUILDFLAG(IS_OZONE)
#include "ui/ozone/public/ozone_platform.h"
#endif

// Only testing for V4L2 devices since we have no devices that need image
// processing and use VA-API.
#if !BUILDFLAG(USE_V4L2_CODEC)
#error "Only V4L2 implemented."
#endif

static constexpr int kMM21TileWidth = 32;
static constexpr int kMM21TileHeight = 16;

static constexpr int kNumberOfTestFrames = 10;
static constexpr int kNumberOfTestCycles = 200;
static constexpr int kNumberOfCappedTestCycles = 200;

static constexpr int kTestImageWidth = 1920;
static constexpr int kTestImageHeight = 1088;

static constexpr int kRandomFrameSeed = 1000;

static constexpr int kUsecPerFrameAt60fps = 16666;

static constexpr int kUpScalingOutputWidth = 1920;
static constexpr int kUpScalingOutputHeight = 1080;

static constexpr int kDownScalingOutputWidth = 480;
static constexpr int kDownScalingOutputHeight = 270;

namespace media {
namespace {

const char* usage_msg =
    "usage: image_processor_perftest\n"
    "[--gtest_help] [--help] [-v=<level>] [--vmodule=<config>] \n";

const char* help_msg =
    "Run the image processor perf tests.\n\n"
    "The following arguments are supported:\n"
    "  --gtest_help          display the gtest help and exit.\n"
    "  --help                display this help and exit.\n"
    "  --source_directory    specify test input files location.\n"
    "  --output_directory    specify test output file location. Defaults to "
    "                        execution directory if not specified.\n"
    "   -v                   enable verbose mode, e.g. -v=2.\n"
    "  --vmodule             enable verbose mode for the specified module.\n";

media::test::VideoTestEnvironment* g_env;

// Default output folder used to store performance metrics.
base::FilePath g_output_directory =
    base::FilePath(base::FilePath::kCurrentDirectory);

base::FilePath g_source_directory =
    base::FilePath(base::FilePath::kCurrentDirectory);

base::FilePath BuildSourceFilePath(const base::FilePath& filename) {
  return media::g_source_directory.Append(filename);
}

constexpr char kNV12Image720P[] =
    FILE_PATH_LITERAL("puppets-1280x720.nv12.yuv");

bool SupportsNecessaryGLExtension() {
  scoped_refptr<gl::GLSurface> gl_surface =
      gl::init::CreateOffscreenGLSurface(gl::GetDefaultDisplay(), gfx::Size());
  if (!gl_surface) {
    LOG(ERROR) << "Error creating GL surface";
    return false;
  }
  scoped_refptr<gl::GLContext> gl_context = gl::init::CreateGLContext(
      nullptr, gl_surface.get(), gl::GLContextAttribs());
  if (!gl_context) {
    LOG(ERROR) << "Error creating GL context";
    return false;
  }
  ui::ScopedMakeCurrent current_context(gl_context.get(), gl_surface.get());
  if (!current_context.IsContextCurrent()) {
    LOG(ERROR) << "Error making GL context current";
    return false;
  }

  return gl_context->HasExtension("GL_EXT_YUV_target");
}
scoped_refptr<VideoFrame> CreateNV12Frame(const gfx::Size& size,
                                          VideoFrame::StorageType type) {
  const gfx::Rect visible_rect(size);
  constexpr base::TimeDelta kNullTimestamp;
  if (type == VideoFrame::STORAGE_GPU_MEMORY_BUFFER) {
    return CreateGpuMemoryBufferVideoFrame(
        VideoPixelFormat::PIXEL_FORMAT_NV12, size, visible_rect, size,
        kNullTimestamp, gfx::BufferUsage::SCANOUT_CPU_READ_WRITE);
  } else if (type == VideoFrame::STORAGE_DMABUFS) {
    return CreatePlatformVideoFrame(VideoPixelFormat::PIXEL_FORMAT_NV12, size,
                                    visible_rect, size, kNullTimestamp,
                                    gfx::BufferUsage::SCANOUT_CPU_READ_WRITE);
  } else {
    DCHECK_EQ(type, VideoFrame::STORAGE_OWNED_MEMORY);
    return VideoFrame::CreateFrame(VideoPixelFormat::PIXEL_FORMAT_NV12, size,
                                   visible_rect, size, kNullTimestamp);
  }
}

scoped_refptr<VideoFrame> CreateRandomMM21Frame(const gfx::Size& size,
                                                VideoFrame::StorageType type) {
  DCHECK_EQ(size.width(), base::bits::AlignUpDeprecatedDoNotUse(
                              size.width(), kMM21TileWidth));
  DCHECK_EQ(size.height(), base::bits::AlignUpDeprecatedDoNotUse(
                               size.height(), kMM21TileHeight));

  scoped_refptr<VideoFrame> frame = CreateNV12Frame(size, type);
  if (!frame) {
    LOG(ERROR) << "Failed to create MM21 frame";
    return nullptr;
  }

  scoped_refptr<VideoFrame> mapped_frame;
  if (type != VideoFrame::STORAGE_OWNED_MEMORY) {
    // The MM21 path only makes sense for V4L2, so we should never get an Intel
    // media compressed buffer here.
    CHECK(!IsIntelMediaCompressedModifier(frame->layout().modifier()));
    std::unique_ptr<VideoFrameMapper> frame_mapper =
        VideoFrameMapperFactory::CreateMapper(
            VideoPixelFormat::PIXEL_FORMAT_NV12, type,
            /*force_linear_buffer_mapper=*/true,
            /*must_support_intel_media_compressed_buffers=*/false);
    if (!frame_mapper) {
      LOG(ERROR) << "Unable to create a VideoFrameMapper";
      return nullptr;
    }
    mapped_frame = frame_mapper->Map(frame, PROT_READ | PROT_WRITE);
    if (!mapped_frame) {
      LOG(ERROR) << "Unable to map MM21 frame";
      return nullptr;
    }
  } else {
    mapped_frame = frame;
  }

  uint8_t* y_plane_ptr =
      mapped_frame->GetWritableVisibleData(VideoFrame::Plane::kY);
  const auto y_plane_stride = mapped_frame->stride(VideoFrame::Plane::kY);
  base::span<uint8_t> y_plane =
      // TODO(crbug.com/338570700): VideoFrame should return spans instead of
      // unbounded pointers.
      UNSAFE_BUFFERS(base::span(
          y_plane_ptr,
          y_plane_stride *
              base::checked_cast<size_t>(mapped_frame->coded_size().height())));

  uint8_t* uv_plane_ptr =
      mapped_frame->GetWritableVisibleData(VideoFrame::Plane::kUV);
  const auto uv_plane_stride = mapped_frame->stride(VideoFrame::Plane::kUV);
  base::span<uint8_t> uv_plane =
      // TODO(crbug.com/338570700): VideoFrame should return spans instead of
      // unbounded pointers. Note: Elsewhere the `height / 2` is rounded up, but
      // here it is not.
      UNSAFE_BUFFERS(base::span(
          uv_plane_ptr,
          uv_plane_stride *
              base::checked_cast<size_t>(mapped_frame->coded_size().height()) /
              2u));
  const auto width =
      base::checked_cast<size_t>(mapped_frame->coded_size().width());

  for (int row = 0; row < size.height(); row++) {
    auto [row_bytes, rem] = y_plane.split_at(y_plane_stride);
    base::RandBytes(row_bytes.first(width));
    y_plane = rem;
  }
  for (int row = 0; row < size.height() / 2; row++) {
    auto [row_bytes, rem] = uv_plane.split_at(uv_plane_stride);
    base::RandBytes(row_bytes.first(width));
    uv_plane = rem;
  }

  return frame;
}

void WriteJsonResult(std::vector<std::pair<std::string, double>> data) {
  base::Value::Dict metrics;
  for (auto i : data) {
    metrics.Set(i.first, i.second);
  }

  const auto output_folder_path = base::FilePath(g_output_directory);
  std::string metrics_str;
  ASSERT_TRUE(base::JSONWriter::WriteWithOptions(
      metrics, base::JSONWriter::OPTIONS_PRETTY_PRINT, &metrics_str));
  const base::FilePath metrics_file_path = output_folder_path.Append(
      g_env->GetTestOutputFilePath().AddExtension(FILE_PATH_LITERAL(".json")));
  // Make sure that the directory into which json is saved is created.
  LOG_ASSERT(base::CreateDirectory(metrics_file_path.DirName()));
  base::File metrics_output_file(
      base::FilePath(metrics_file_path),
      base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
  const int bytes_written = metrics_output_file.WriteAtCurrentPos(
      metrics_str.data(), metrics_str.length());
  ASSERT_EQ(bytes_written, static_cast<int>(metrics_str.length()));
  LOG(INFO) << "Wrote performance metrics to: " << metrics_file_path;
}

class ImageProcessorPerfTest : public ::testing::Test {
 public:
  enum TestType {
    kMM21Detiling,
    kNV12Downscaling,
    kNV12Upscaling,
  };

  void InitializeImageProcessorTest(bool use_cpu_memory, TestType test_type) {
    test_image_size_.SetSize(kTestImageWidth, kTestImageHeight);
    ASSERT_TRUE((test_image_size_.width() % kMM21TileWidth) == 0);
    ASSERT_TRUE((test_image_size_.height() % kMM21TileHeight) == 0);

    test_image_visible_rect_.set_size(test_image_size_);

    candidate_.size = test_image_size_;
    candidate_.fourcc = test_type == kMM21Detiling ? Fourcc(Fourcc::MM21)
                                                   : Fourcc(Fourcc::NV12);
    candidates_ = {candidate_};

    scoped_refptr<base::SequencedTaskRunner> client_task_runner_ =
        base::SequencedTaskRunner::GetCurrentDefault();
    base::RepeatingClosure quit_closure_ = run_loop_.QuitClosure();
    bool image_processor_error_ = false;

    input_frames_.reserve(kNumberOfTestFrames);
    for (int i = 0; i < kNumberOfTestFrames; i++) {
      input_frames_.push_back(CreateRandomMM21Frame(
          test_image_size_, use_cpu_memory ? VideoFrame::STORAGE_OWNED_MEMORY
                                           : VideoFrame::STORAGE_DMABUFS));
    }

    gfx::Size output_size = test_image_size_;
    if (test_type == kNV12Downscaling) {
      output_size =
          gfx::Size(kDownScalingOutputWidth, kDownScalingOutputHeight);
    } else if (test_type == kNV12Upscaling) {
      output_size = gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight);
    }

    ASSERT_EQ(test_type == kMM21Detiling, output_size == test_image_size_);
    output_frame_ =
        CreateNV12Frame(output_size, VideoFrame::STORAGE_GPU_MEMORY_BUFFER);
    ASSERT_TRUE(output_frame_) << "Error creating output frame";

    error_cb_ = base::BindRepeating(
        [](scoped_refptr<base::SequencedTaskRunner> client_task_runner_,
           base::RepeatingClosure quit_closure_, bool* image_processor_error_) {
          CHECK(client_task_runner_->RunsTasksInCurrentSequence());
          ASSERT_TRUE(false);
          quit_closure_.Run();
        },
        client_task_runner_, quit_closure_, &image_processor_error_);

    pick_format_cb_ = base::BindRepeating(
        [](const std::vector<Fourcc>&, std::optional<Fourcc>) {
          return std::make_optional<Fourcc>(Fourcc::NV12);
        });
  }

  void InitializeInputImage(bool use_cpu_memory) {
    std::unique_ptr<test::Image> input_image = std::make_unique<test::Image>(
        BuildSourceFilePath(base::FilePath(kNV12Image720P)));
    ASSERT_TRUE(input_image->Load());
    ASSERT_TRUE(input_image->LoadMetadata());
    const auto input_layout = test::CreateVideoFrameLayout(
        input_image->PixelFormat(), input_image->Size());
    ASSERT_TRUE(input_layout) << "Error creating input layout.";

    scoped_refptr<const VideoFrame> tmp_video_frame =
        CreateVideoFrameFromImage(*input_image);
    ASSERT_TRUE(tmp_video_frame);

    input_image_frame_ = test::CloneVideoFrame(
        tmp_video_frame.get(), *input_layout,
        use_cpu_memory ? VideoFrame::STORAGE_OWNED_MEMORY
                       : VideoFrame::STORAGE_GPU_MEMORY_BUFFER,
        gfx::BufferUsage::SCANOUT_CPU_READ_WRITE);
    ASSERT_TRUE(input_image_frame_) << "Error creating input frame.";

    candidate_.fourcc = Fourcc(Fourcc::NV12);
    candidate_.size = input_image->Size();
    candidates_ = {candidate_};
  }

  gfx::Size test_image_size_;
  gfx::Rect test_image_visible_rect_;
  ImageProcessor::PixelLayoutCandidate candidate_{Fourcc(Fourcc::MM21),
                                                  gfx::Size()};
  std::vector<ImageProcessor::PixelLayoutCandidate> candidates_;
  base::RunLoop run_loop_;
  std::vector<scoped_refptr<VideoFrame>> input_frames_;
  scoped_refptr<VideoFrame> output_frame_;
  ImageProcessor::ErrorCB error_cb_;
  ImageProcessorFactory::PickFormatCB pick_format_cb_;
  scoped_refptr<VideoFrame> input_image_frame_;
};

// Tests GLImageProcessor by feeding in |kNumberOfTestFrames| unique input
// frames looped over |kNumberOfTestCycles| iterations to the GLImageProcessor
// as fast as possible. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, UncappedGLImageProcessorPerfTest) {
  if (!SupportsNecessaryGLExtension()) {
    GTEST_SKIP() << "Skipping GL Backend test, unsupported platform.";
  }

  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kMM21Detiling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();
  std::unique_ptr<ImageProcessor> gl_image_processor = ImageProcessorFactory::
      CreateGLImageProcessorWithInputCandidatesForTesting(
          candidates_, test_image_visible_rect_, test_image_size_,
          /*num_buffers=*/1, client_task_runner, pick_format_cb_, error_cb_);
  ASSERT_TRUE(gl_image_processor) << "Error creating GLImageProcessor";

  LOG(INFO) << "Running GLImageProcessor Uncapped Perf Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB gl_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    gl_image_processor->Process(input_frames_[num_cycles % kNumberOfTestFrames],
                                output_frame_, std::move(gl_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

// Tests the LibYUV by feeding in |kNumberOfTestFrames| unique input
// frames looped over |kNumberOfTestCycles| iterations to the LibYUV
// as fast as possible. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, UncappedLibYUVPerfTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/true, kMM21Detiling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();

  std::unique_ptr<ImageProcessor> libyuv_image_processor =
      ImageProcessorFactory::
          CreateLibYUVImageProcessorWithInputCandidatesForTesting(
              candidates_, test_image_visible_rect_, test_image_size_,
              /*num_buffers=*/1, client_task_runner, pick_format_cb_,
              error_cb_);
  ASSERT_TRUE(libyuv_image_processor) << "Error creating LibYUV";

  LOG(INFO) << "Running LibYUV Uncapped Perf Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB libyuv_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    libyuv_image_processor->Process(
        input_frames_[num_cycles % kNumberOfTestFrames], output_frame_,
        std::move(libyuv_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  // Preventing integer division inaccuracies with |delta_time|.
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicroseconds()},
                   {"FramesPerSecond", fps}});
}

// Tests GLImageProcessor by feeding in |kNumberOfTestFrames| unique input
// frames looped over |kNumberOfCappedTestCycles| iterations to the
// GLImageProcessor at 60fps. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, CappedGLImageProcessorPerfTest) {
  if (!SupportsNecessaryGLExtension()) {
    GTEST_SKIP() << "Skipping GL Backend test, unsupported platform.";
  }

  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kMM21Detiling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();
  std::unique_ptr<ImageProcessor> gl_image_processor = ImageProcessorFactory::
      CreateGLImageProcessorWithInputCandidatesForTesting(
          candidates_, test_image_visible_rect_, test_image_size_,
          /*num_buffers=*/1, client_task_runner, pick_format_cb_, error_cb_);
  ASSERT_TRUE(gl_image_processor) << "Error creating GLImageProcessor";

  LOG(INFO) << "Running GLImageProcessor Capped Perf Test";
  int loop_iterations = kNumberOfCappedTestCycles;
  base::TimeTicks start, end;
  base::RepeatingCallback<void(scoped_refptr<VideoFrame>)>* gl_callback_ptr;
  base::RepeatingCallback gl_callback =
      base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
        CHECK(client_task_runner->RunsTasksInCurrentSequence());

        if (!(--loop_iterations)) {
          quit_closure.Run();
        } else {
          end = base::TimeTicks::Now();
          base::TimeDelta delta_time = end - start;
          if (delta_time.InMicroseconds() < kUsecPerFrameAt60fps) {
            usleep(kUsecPerFrameAt60fps - delta_time.InMicroseconds());
          } else {
            LOG(WARNING) << "Frame detiling was late by "
                         << (delta_time.InMicroseconds() - kUsecPerFrameAt60fps)
                         << "us";
          }
          start = base::TimeTicks::Now();
          gl_image_processor->Process(
              input_frames_[loop_iterations % kNumberOfTestFrames],
              output_frame_, *gl_callback_ptr);
        }
      });

  gl_callback_ptr = &gl_callback;

  gl_image_processor->Process(input_frames_[0], output_frame_,
                              *gl_callback_ptr);

  start = base::TimeTicks::Now();
  auto total_start_time = base::TimeTicks::Now();
  run_loop_.Run();

  auto total_end_time = base::TimeTicks::Now();
  base::TimeDelta total_delta_time = total_end_time - total_start_time;
  const double fps =
      (kNumberOfCappedTestCycles / total_delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfCappedTestCycles},
                   {"TotalDurationMs", total_delta_time.InMicroseconds()},
                   {"FramesPerSecond", fps}});
}

// Tests LibYUV by feeding in |kNumberOfTestFrames| unique input
// frames looped over |kNumberOfCappedTestCycles| iterations to the
// LibYUV at 60fps. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, CappedLibYUVPerfTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/true, kMM21Detiling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();
  std::unique_ptr<ImageProcessor> libyuv_image_processor =
      ImageProcessorFactory::
          CreateLibYUVImageProcessorWithInputCandidatesForTesting(
              candidates_, test_image_visible_rect_, test_image_size_,
              /*num_buffers=*/1, client_task_runner, pick_format_cb_,
              error_cb_);
  ASSERT_TRUE(libyuv_image_processor) << "Error creating LibYUV";

  LOG(INFO) << "Running LibYUV Capped Perf Test";
  int loop_iterations = 0;
  base::TimeTicks start, end;
  base::RepeatingCallback<void(scoped_refptr<VideoFrame>)>* libyuv_callback_ptr;
  base::RepeatingCallback libyuv_callback =
      base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
        CHECK(client_task_runner->RunsTasksInCurrentSequence());

        if ((++loop_iterations) >= kNumberOfCappedTestCycles) {
          quit_closure.Run();
        } else {
          end = base::TimeTicks::Now();
          base::TimeDelta delta_time = end - start;
          if (delta_time.InMicroseconds() < kUsecPerFrameAt60fps) {
            usleep(kUsecPerFrameAt60fps - delta_time.InMicroseconds());
          } else {
            LOG(WARNING) << "Frame detiling was late by "
                         << (delta_time.InMicroseconds() - kUsecPerFrameAt60fps)
                         << "us";
          }
          start = base::TimeTicks::Now();
          libyuv_image_processor->Process(
              input_frames_[loop_iterations % kNumberOfTestFrames],
              output_frame_, *libyuv_callback_ptr);
        }
      });

  libyuv_callback_ptr = &libyuv_callback;

  libyuv_image_processor->Process(input_frames_[0], output_frame_,
                                  *libyuv_callback_ptr);

  start = base::TimeTicks::Now();
  auto total_start_time = base::TimeTicks::Now();
  run_loop_.Run();

  auto total_end_time = base::TimeTicks::Now();
  base::TimeDelta total_delta_time = total_end_time - total_start_time;
  const double fps =
      (kNumberOfCappedTestCycles / total_delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfCappedTestCycles},
                   {"TotalDurationMs", total_delta_time.InMicroseconds()},
                   {"FramesPerSecond", fps}});
}

// Tests the GLImageProcessor by feeding in a 1280x720 NV12 input frame and
// scaling it up to 1920x1080 and then scaling it back down to its original
// size. Will print out the PSNR calculation for each plane and verify that
// the PSNR values are greater than 40.0.
TEST_F(ImageProcessorPerfTest, GLNV12ScalingComparisonTest) {
  InitializeInputImage(/*use_cpu_memory=*/false);

  base::RunLoop run_loop;
  auto client_task_runner = base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop.QuitClosure();
  bool image_processor_error = false;
  ImageProcessor::ErrorCB error_cb = base::BindRepeating(
      [](scoped_refptr<base::SequencedTaskRunner> client_task_runner,
         base::RepeatingClosure quit_closure, bool* image_processor_error) {
        CHECK(client_task_runner->RunsTasksInCurrentSequence());
        *image_processor_error = true;
        quit_closure.Run();
      },
      client_task_runner, quit_closure, &image_processor_error);
  ImageProcessorFactory::PickFormatCB pick_format_cb = base::BindRepeating(
      [](const std::vector<Fourcc>&, std::optional<Fourcc>) {
        return std::make_optional<Fourcc>(Fourcc::NV12);
      });

  std::unique_ptr<ImageProcessor> gl_upscaling_image_processor =
      ImageProcessorFactory::
          CreateGLImageProcessorWithInputCandidatesForTesting(
              candidates_, gfx::Rect(input_image_frame_->coded_size()),
              gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight),
              /*num_buffers=*/1, client_task_runner, pick_format_cb, error_cb);
  ASSERT_TRUE(gl_upscaling_image_processor)
      << "Error creating GLImageProcessor";

  const ImageProcessor::PixelLayoutCandidate downscaling_candidate = {
      Fourcc(Fourcc::NV12),
      gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight)};
  std::vector<ImageProcessor::PixelLayoutCandidate> downscaling_candidates = {
      downscaling_candidate};

  std::unique_ptr<ImageProcessor> gl_downscaling_image_processor =
      ImageProcessorFactory::
          CreateGLImageProcessorWithInputCandidatesForTesting(
              downscaling_candidates,
              gfx::Rect(kUpScalingOutputWidth, kUpScalingOutputHeight),
              input_image_frame_->coded_size(),
              /*num_buffers=*/1, client_task_runner, pick_format_cb, error_cb);
  ASSERT_TRUE(gl_downscaling_image_processor)
      << "Error creating GLImageProcessor";

  scoped_refptr<VideoFrame> gl_upscaling_output_frame =
      CreateNV12Frame(gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight),
                      VideoFrame::STORAGE_GPU_MEMORY_BUFFER);
  ASSERT_TRUE(gl_upscaling_output_frame) << "Error creating GL output frame";

  scoped_refptr<VideoFrame> gl_downscaling_output_frame = CreateNV12Frame(
      input_image_frame_->coded_size(), VideoFrame::STORAGE_GPU_MEMORY_BUFFER);
  ASSERT_TRUE(gl_downscaling_output_frame) << "Error creating GL output frame";

  ImageProcessor::FrameReadyCB gl_callback2 =
      base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
        CHECK(client_task_runner->RunsTasksInCurrentSequence());
        gl_downscaling_output_frame = std::move(frame);
        quit_closure.Run();
      });

  ImageProcessor::FrameReadyCB gl_callback1 =
      base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
        CHECK(client_task_runner->RunsTasksInCurrentSequence());
        gl_downscaling_image_processor->Process(
            frame, gl_downscaling_output_frame, std::move(gl_callback2));
      });

  gl_upscaling_image_processor->Process(
      input_image_frame_, gl_upscaling_output_frame, std::move(gl_callback1));
  run_loop.Run();

  // The image processor perf tests are currently only available with V4L2, and
  // we should never get Intel media compressed buffers in V4L2 platforms.
  ASSERT_FALSE(IsIntelMediaCompressedModifier(
      gl_downscaling_output_frame->layout().modifier()));
  ASSERT_FALSE(
      IsIntelMediaCompressedModifier(input_image_frame_->layout().modifier()));

  const std::unique_ptr<VideoFrameMapper> output_frame_mapper =
      VideoFrameMapperFactory::CreateMapper(
          PIXEL_FORMAT_NV12, VideoFrame::STORAGE_GPU_MEMORY_BUFFER,
          /*force_linear_buffer_mapper=*/true,
          /*must_support_intel_media_compressed_buffers=*/false);
  ASSERT_TRUE(output_frame_mapper);

  const scoped_refptr<VideoFrame> mapped_gl_output = output_frame_mapper->Map(
      gl_downscaling_output_frame, PROT_READ | PROT_WRITE);
  ASSERT_TRUE(mapped_gl_output);

  const scoped_refptr<VideoFrame> mapped_input_frame =
      output_frame_mapper->Map(input_image_frame_, PROT_READ | PROT_WRITE);
  ASSERT_TRUE(mapped_input_frame);

  const gfx::Size image_size = mapped_gl_output->visible_rect().size();
  const gfx::Size half_image_size((image_size.width() + 1) / 2,
                                  (image_size.height() + 1) / 2);

  uint8_t* out_y = static_cast<uint8_t*>(
      mmap(nullptr, image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t* out_u = static_cast<uint8_t*>(
      mmap(nullptr, half_image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t* out_v = static_cast<uint8_t*>(
      mmap(nullptr, half_image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  libyuv::NV12ToI420(
      mapped_gl_output->visible_data(0), mapped_gl_output->stride(0),
      mapped_gl_output->visible_data(1), mapped_gl_output->stride(1), out_y,
      image_size.width(), out_u, half_image_size.width(), out_v,
      half_image_size.width(), image_size.width(), image_size.height());

  uint8_t* input_y = static_cast<uint8_t*>(
      mmap(nullptr, image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t* input_u = static_cast<uint8_t*>(
      mmap(nullptr, half_image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
  uint8_t* input_v = static_cast<uint8_t*>(
      mmap(nullptr, half_image_size.GetArea(), PROT_READ | PROT_WRITE,
           MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));

  libyuv::NV12ToI420(
      mapped_input_frame->visible_data(0), mapped_input_frame->stride(0),
      mapped_input_frame->visible_data(1), mapped_input_frame->stride(1),
      input_y, image_size.width(), input_u, half_image_size.width(), input_v,
      half_image_size.width(), image_size.width(), image_size.height());

  double psnr_test = libyuv::I420Psnr(
      out_y, image_size.width(), out_u, half_image_size.width(), out_v,
      half_image_size.width(), input_y, image_size.width(), input_u,
      half_image_size.width(), input_v, half_image_size.width(),
      image_size.width(), image_size.height());

  munmap(out_y, image_size.GetArea());
  munmap(out_u, half_image_size.GetArea());
  munmap(out_v, half_image_size.GetArea());
  munmap(input_y, image_size.GetArea());
  munmap(input_u, half_image_size.GetArea());
  munmap(input_v, half_image_size.GetArea());

  perf_test::PerfResultReporter reporter("GLImageProcessor NV12 Scaling",
                                         "PSNR Test");
  reporter.RegisterImportantMetric(".PSNR", "decibels");
  reporter.AddResult(".PSNR", psnr_test);

  WriteJsonResult({{"PSNR", psnr_test}});
}

// Tests GLImageProcessor by feeding in |kNumberOfTestFrames| unique NV12 input
// frames looped over |kNumberOfTestCycles| iterations to the GLImageProcessor
// to be downscaled to 480x270. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, GLImageProcessorNV12DownscalingTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kNV12Downscaling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();
  std::unique_ptr<ImageProcessor> gl_image_processor = ImageProcessorFactory::
      CreateGLImageProcessorWithInputCandidatesForTesting(
          candidates_, test_image_visible_rect_,
          gfx::Size(kDownScalingOutputWidth, kDownScalingOutputHeight),
          /*num_buffers=*/1, client_task_runner, pick_format_cb_, error_cb_);
  ASSERT_TRUE(gl_image_processor) << "Error creating GLImageProcessor";

  LOG(INFO) << "Running GLImageProcessor NV12 Downscaling Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB gl_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    gl_image_processor->Process(input_frames_[num_cycles % kNumberOfTestFrames],
                                output_frame_, std::move(gl_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

// Tests GLImageProcessor by feeding in |kNumberOfTestFrames| unique NV12 input
// frames looped over |kNumberOfTestCycles| iterations to the GLImageProcessor
// to be upscaled to 1920x1080. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, GLImageProcessorNV12UpscalingTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kNV12Upscaling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();
  std::unique_ptr<ImageProcessor> gl_image_processor = ImageProcessorFactory::
      CreateGLImageProcessorWithInputCandidatesForTesting(
          candidates_, test_image_visible_rect_,
          gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight),
          /*num_buffers=*/1, client_task_runner, pick_format_cb_, error_cb_);
  ASSERT_TRUE(gl_image_processor) << "Error creating GLImageProcessor";

  LOG(INFO) << "Running GLImageProcessor NV12 Upscaling Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB gl_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    gl_image_processor->Process(input_frames_[num_cycles % kNumberOfTestFrames],
                                output_frame_, std::move(gl_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

// Tests LibYUV by feeding in |kNumberOfTestFrames| unique NV12 input
// frames looped over |kNumberOfTestCycles| iterations to the LibYUV
// to be downscaled to 480x270. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, LibYUVNV12DownscalingTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kNV12Downscaling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();

  std::unique_ptr<ImageProcessor> libyuv_image_processor =
      ImageProcessorFactory::
          CreateLibYUVImageProcessorWithInputCandidatesForTesting(
              candidates_, test_image_visible_rect_,
              gfx::Size(kDownScalingOutputWidth, kDownScalingOutputHeight),
              /*num_buffers=*/1, client_task_runner, pick_format_cb_,
              error_cb_);
  ASSERT_TRUE(libyuv_image_processor) << "Error creating LibYUV";

  LOG(INFO) << "Running LibYUV NV12 Downscaling Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB libyuv_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    libyuv_image_processor->Process(
        input_frames_[num_cycles % kNumberOfTestFrames], output_frame_,
        std::move(libyuv_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  // Preventing integer division inaccuracies with |delta_time|.
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

// Tests LibYUV by feeding in |kNumberOfTestFrames| unique NV12 input
// frames looped over |kNumberOfTestCycles| iterations to the LibYUV
// to be upscaled to 1920x1080. Will print out elapsed processing time.
TEST_F(ImageProcessorPerfTest, LibYUVNV12UpscalingTest) {
  InitializeImageProcessorTest(/*use_cpu_memory=*/false, kNV12Upscaling);

  scoped_refptr<base::SequencedTaskRunner> client_task_runner =
      base::SequencedTaskRunner::GetCurrentDefault();
  base::RepeatingClosure quit_closure = run_loop_.QuitClosure();

  std::unique_ptr<ImageProcessor> libyuv_image_processor =
      ImageProcessorFactory::
          CreateLibYUVImageProcessorWithInputCandidatesForTesting(
              candidates_, test_image_visible_rect_,
              gfx::Size(kUpScalingOutputWidth, kUpScalingOutputHeight),
              /*num_buffers=*/1, client_task_runner, pick_format_cb_,
              error_cb_);
  ASSERT_TRUE(libyuv_image_processor) << "Error creating LibYUV";

  LOG(INFO) << "Running LibYUV NV12 Upscaling Test";
  int outstanding_processors = kNumberOfTestCycles;
  for (int num_cycles = outstanding_processors; num_cycles > 0; num_cycles--) {
    ImageProcessor::FrameReadyCB libyuv_callback =
        base::BindLambdaForTesting([&](scoped_refptr<VideoFrame> frame) {
          CHECK(client_task_runner->RunsTasksInCurrentSequence());
          if (!(--outstanding_processors)) {
            quit_closure.Run();
          }
        });
    libyuv_image_processor->Process(
        input_frames_[num_cycles % kNumberOfTestFrames], output_frame_,
        std::move(libyuv_callback));
  }

  auto start_time = base::TimeTicks::Now();
  run_loop_.Run();
  auto end_time = base::TimeTicks::Now();
  base::TimeDelta delta_time = end_time - start_time;
  // Preventing integer division inaccuracies with |delta_time|.
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

#if BUILDFLAG(ENABLE_VULKAN)

class VulkanImageProcessorPerfTest
    : public ImageProcessorPerfTest,
      public testing::WithParamInterface<TiledImageFormat> {
 public:
  VulkanImageProcessorPerfTest() = default;
  ~VulkanImageProcessorPerfTest() = default;

  struct PrintToStringParamName {
    template <class ParamType>
    std::string operator()(
        const testing::TestParamInfo<ParamType>& info) const {
      return base::StringPrintf("%s", (info.param == kMM21) ? "MM21" : "MT2T");
    }
  };
};

TEST_P(VulkanImageProcessorPerfTest, Detile) {
  const bool is_10bit = GetParam() == kMT2T;
  const size_t bpp_numerator = is_10bit ? 5 : 1;
  const size_t bpp_denom = is_10bit ? 4 : 1;
  const VideoPixelFormat out_video_format =
      is_10bit ? VideoPixelFormat::PIXEL_FORMAT_XR30
               : VideoPixelFormat::PIXEL_FORMAT_ARGB;
  const viz::SharedImageFormat out_viz_format =
      is_10bit ? viz::SinglePlaneFormat::kBGRA_1010102
               : viz::SinglePlaneFormat::kBGRA_8888;

  // Initialize shared image infrastructure.
  auto share_group = base::MakeRefCounted<gl::GLShareGroup>();
  auto surface =
      gl::init::CreateOffscreenGLSurface(gl::GetDefaultDisplay(), gfx::Size());
  auto context = gl::init::CreateGLContext(share_group.get(), surface.get(),
                                           gl::GLContextAttribs());
  context->MakeCurrent(surface.get());
  auto context_state = base::MakeRefCounted<gpu::SharedContextState>(
      share_group, surface, context, false, base::DoNothing(),
      gpu::GpuPreferences().gr_context_type);
  gpu::SharedImageManager shared_image_manager;
  gpu::GpuPreferences gpu_preferences;
  gpu::GpuDriverBugWorkarounds gpu_workarounds;
  gpu::GpuFeatureInfo gpu_info;
  gpu::SharedImageFactory shared_image_factory(
      gpu_preferences, gpu_workarounds, gpu_info, context_state.get(),
      &shared_image_manager, nullptr, false);

  gfx::Size test_image_size(kTestImageWidth, kTestImageHeight);
  gfx::Size test_coded_size(base::bits::AlignUpDeprecatedDoNotUse(
                                test_image_size.width(), kMM21TileWidth),
                            base::bits::AlignUpDeprecatedDoNotUse(
                                test_image_size.height(), kMM21TileHeight));
  std::vector<scoped_refptr<VideoFrame>> input_frames(kNumberOfTestFrames);
  std::vector<scoped_refptr<VideoFrame>> output_frames(kNumberOfTestFrames);
  std::vector<gpu::Mailbox> input_mailboxes(kNumberOfTestFrames);
  std::vector<gpu::Mailbox> output_mailboxes(kNumberOfTestFrames);

  constexpr base::TimeDelta kNullTimestamp;
  viz::SharedImageFormat format_nv12 = viz::SharedImageFormat::MultiPlane(
      viz::SharedImageFormat::PlaneConfig::kY_UV,
      viz::SharedImageFormat::Subsampling::k420,
      viz::SharedImageFormat::ChannelFormat::k8);
  format_nv12.SetPrefersExternalSampler();
  for (size_t i = 0; i < kNumberOfTestFrames; i++) {
    input_frames[i] = CreateRandomMM21Frame(
        gfx::Size(test_image_size.width(),
                  test_image_size.height() * bpp_numerator / bpp_denom),
        VideoFrame::STORAGE_DMABUFS);
    input_mailboxes[i] = gpu::Mailbox::GenerateForSharedImage();
    auto input_gmb = CreateGpuMemoryBufferHandle(input_frames[i].get());
    shared_image_factory.CreateSharedImage(
        input_mailboxes[i], format_nv12, input_frames[i]->coded_size(),
        gfx::ColorSpace::CreateSRGB(), kTopLeft_GrSurfaceOrigin,
        kOpaque_SkAlphaType,
        gpu::SharedImageUsage::SHARED_IMAGE_USAGE_DISPLAY_READ, "TestLabel",
        std::move(input_gmb));

    output_frames[i] = CreateGpuMemoryBufferVideoFrame(
        out_video_format, test_coded_size, gfx::Rect(test_image_size),
        test_coded_size, kNullTimestamp,
        gfx::BufferUsage::SCANOUT_CPU_READ_WRITE);
    output_mailboxes[i] = gpu::Mailbox::GenerateForSharedImage();
    auto output_gmb = CreateGpuMemoryBufferHandle(output_frames[i].get());
    shared_image_factory.CreateSharedImage(
        output_mailboxes[i], out_viz_format, test_coded_size,
        gfx::ColorSpace::CreateSRGB(), kTopLeft_GrSurfaceOrigin,
        kUnpremul_SkAlphaType,
        gpu::SharedImageUsage::SHARED_IMAGE_USAGE_DISPLAY_WRITE, "TestLabel",
        std::move(output_gmb));
  }

  auto vulkan_image_processor =
      VulkanImageProcessor::Create(/*is_protected=*/false, GetParam());
  ASSERT_TRUE(vulkan_image_processor);

  auto start_time = base::TimeTicks::Now();
  for (int i = 0; i < kNumberOfTestCycles; i++) {
    auto input_representation = shared_image_manager.ProduceVulkan(
        input_mailboxes[i % kNumberOfTestFrames], nullptr,
        vulkan_image_processor->GetVulkanDeviceQueue(),
        vulkan_image_processor->GetVulkanImplementation());
    auto output_representation = shared_image_manager.ProduceVulkan(
        output_mailboxes[i % kNumberOfTestFrames], nullptr,
        vulkan_image_processor->GetVulkanDeviceQueue(),
        vulkan_image_processor->GetVulkanImplementation());

    {
      std::vector<VkSemaphore> begin_semaphores;
      std::vector<VkSemaphore> end_semaphores;
      auto input_access = input_representation->BeginScopedAccess(
          gpu::RepresentationAccessMode::kRead, begin_semaphores,
          end_semaphores);
      auto output_access = output_representation->BeginScopedAccess(
          gpu::RepresentationAccessMode::kWrite, begin_semaphores,
          end_semaphores);

      // TODO(b/251458823): Add tests for more interesting crop and rotation
      // parameters. Preliminary testing indicates that rotation in particular
      // might have a substantial impact on performance.
      vulkan_image_processor->Process(
          input_access->GetVulkanImage(), test_image_size,
          output_access->GetVulkanImage(),
          gfx::RectF(static_cast<float>(test_coded_size.width()),
                     static_cast<float>(test_coded_size.height())),
          gfx::RectF(1.0f, 1.0f), gfx::OVERLAY_TRANSFORM_NONE, begin_semaphores,
          end_semaphores);
    }
  }
  // This implicitly waits for all semaphores to signal.
  vulkan_image_processor->GetVulkanDeviceQueue()
      ->GetFenceHelper()
      ->PerformImmediateCleanup();
  auto end_time = base::TimeTicks::Now();

  base::TimeDelta delta_time = end_time - start_time;
  // Preventing integer division inaccuracies with |delta_time|.
  const double fps = (kNumberOfTestCycles / delta_time.InSecondsF());

  WriteJsonResult({{"FramesDecoded", kNumberOfTestCycles},
                   {"TotalDurationMs", delta_time.InMicrosecondsF()},
                   {"FramesPerSecond", fps}});
}

INSTANTIATE_TEST_SUITE_P(
    ,
    VulkanImageProcessorPerfTest,
    testing::Values(kMM21, kMT2T),
    VulkanImageProcessorPerfTest::PrintToStringParamName());

#endif

}  // namespace
}  // namespace media

int main(int argc, char** argv) {
  base::CommandLine::Init(argc, argv);

  // Print the help message if requested. This needs to be done before
  // initializing gtest, to overwrite the default gtest help message.
  const base::CommandLine* cmd_line = base::CommandLine::ForCurrentProcess();
  LOG_ASSERT(cmd_line);
  if (cmd_line->HasSwitch("help")) {
    std::cout << media::usage_msg << "\n" << media::help_msg;
    return 0;
  }

  base::CommandLine::SwitchMap switches = cmd_line->GetSwitches();
  for (base::CommandLine::SwitchMap::const_iterator it = switches.begin();
       it != switches.end(); ++it) {
    if (it->first.find("gtest_") == 0 ||               // Handled by GoogleTest
        it->first == "v" || it->first == "vmodule") {  // Handled by Chrome
      continue;
    }

    if (it->first == "source_directory") {
      media::g_source_directory = base::FilePath(it->second);
    } else if (it->first == "output_directory") {
      media::g_output_directory = base::FilePath(it->second);
    } else {
      std::cout << "unknown option: --" << it->first << "\n"
                << media::usage_msg;
      return EXIT_FAILURE;
    }
  }
  srand(kRandomFrameSeed);
  testing::InitGoogleTest(&argc, argv);

  auto* const test_environment = new media::test::VideoTestEnvironment;
  media::g_env = reinterpret_cast<media::test::VideoTestEnvironment*>(
      testing::AddGlobalTestEnvironment(test_environment));

// TODO(b/316374371) Try to remove Ozone and replace with EGL and GL.
#if BUILDFLAG(IS_OZONE)
  ui::OzonePlatform::InitParams ozone_param;
  ozone_param.single_process = true;
#if BUILDFLAG(ENABLE_VULKAN) && BUILDFLAG(USE_V4L2_CODEC)
  ui::OzonePlatform::InitializeForUI(ozone_param);
#endif
  ui::OzonePlatform::InitializeForGPU(ozone_param);
#endif

  gl::GLSurfaceTestSupport::InitializeOneOffImplementation(
      gl::GLImplementationParts(gl::kGLImplementationEGLGLES2),
      /*fallback_to_software_gl=*/false);

  return RUN_ALL_TESTS();
}
