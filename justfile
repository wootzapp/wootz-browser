copy_src:
    rsync -av --progress --update ../wootz-browser/src/* ./src/

configure arch rebuild:
    #!/usr/bin/env sh
    if [ "{{rebuild}}" = "yes" ]; then
        echo "Do not configure on rebuild {{arch}}"
        exit 0
    fi
    mkdir -p ./src/out/Debug_{{arch}}
    cat << EOF > ./src/out/Debug_{{arch}}/args.gn
        target_os = "android"
        target_cpu = "{{arch}}"
        is_debug= false
        symbol_level = 0
        enable_nacl = false
        proprietary_codecs = true
        ffmpeg_branding = "Chrome"
        treat_warnings_as_errors=false

        is_java_debug = true
        is_component_build = true
        android_static_analysis = "off"
        cc_wrapper="env CCACHE_SLOPPINESS=time_macros ccache"
    EOF

    cd src && gn gen out/Debug_{{arch}}

build arch="x64" rebuild="yes": (configure arch rebuild)
    cd src && \
    autoninja -C out/Debug_{{arch}} chrome_public_apk

install arch="x64":
    adb install -r src/out/Debug_{{arch}}/apks/WootzApp.apk