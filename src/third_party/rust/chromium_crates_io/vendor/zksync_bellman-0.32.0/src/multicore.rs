//! This is an interface for dealing with the kinds of
//! parallel computations involved in bellman. It's
//! currently just a thin wrapper around CpuPool and
//! crossbeam but may be extended in the future to
//! allow for various parallelism strategies.

extern crate crossbeam;
extern crate futures;
extern crate num_cpus;

use std::future::Future;
use std::pin::Pin;
use std::task::{Context, Poll};

use self::crossbeam::thread::Scope;

use self::futures::channel::oneshot::{channel, Receiver, Sender};
use self::futures::executor::block_on;
use self::futures::executor::ThreadPool;
use self::futures::future::lazy;

#[derive(Clone)]
pub struct Worker {
    pub(crate) cpus: usize,
    pool: ThreadPool,
}

impl Worker {
    // We don't expose this outside the library so that
    // all `Worker` instances have the same number of
    // CPUs configured.

    pub fn new_with_cpus(cpus: usize) -> Worker {
        Worker {
            cpus: cpus,
            pool: ThreadPool::builder().pool_size(cpus).create().expect("should create a thread pool for futures execution"),
        }
    }

    pub fn new() -> Worker {
        Self::new_with_cpus(num_cpus::get_physical())
    }

    pub fn split(&self) -> (Self, Self) {
        let at = self.cpus / 2;
        self.split_at(at)
    }

    pub fn split_at(&self, at: usize) -> (Self, Self) {
        assert!(0 < at && at < self.cpus);

        let first = Self { cpus: at, pool: self.pool.clone() };

        let second = Self {
            cpus: self.cpus - at,
            pool: self.pool.clone(),
        };

        (first, second)
    }

    pub fn log_num_cpus(&self) -> u32 {
        log2_floor(self.cpus)
    }

    pub fn compute<F, T, E>(&self, f: F) -> WorkerFuture<T, E>
    where
        F: FnOnce() -> Result<T, E> + Send + 'static,
        T: Send + 'static,
        E: Send + 'static,
    {
        let (sender, receiver) = channel();
        let lazy_future = lazy(move |_| {
            let res = f();

            if !sender.is_canceled() {
                let _ = sender.send(res);
            }
        });

        let worker_future = WorkerFuture { receiver };

        self.pool.spawn_ok(lazy_future);

        worker_future
    }

    pub fn scope<'a, F, R>(&self, elements: usize, f: F) -> R
    where
        F: FnOnce(&Scope<'a>, usize) -> R,
    {
        let chunk_size = self.get_chunk_size(elements);

        crossbeam::scope(|scope| f(scope, chunk_size)).expect("must run")
    }

    pub fn get_chunk_size(&self, elements: usize) -> usize {
        let chunk_size = if elements <= self.cpus {
            1
        } else {
            Self::chunk_size_for_num_spawned_threads(elements, self.cpus)
        };

        chunk_size
    }

    pub fn get_num_spawned_threads(&self, elements: usize) -> usize {
        let num_spawned = if elements <= self.cpus {
            elements
        } else {
            let chunk = self.get_chunk_size(elements);
            let mut spawned = elements / chunk;
            if spawned * chunk < elements {
                spawned += 1;
            }
            assert!(spawned <= 2 * self.cpus);

            spawned
        };

        num_spawned
    }

    pub fn chunk_size_for_num_spawned_threads(elements: usize, num_threads: usize) -> usize {
        assert!(elements >= num_threads, "received {} elements to spawn {} threads", elements, num_threads);
        if elements % num_threads == 0 {
            elements / num_threads
        } else {
            elements / num_threads + 1
        }
    }
}

pub struct WorkerFuture<T, E> {
    receiver: Receiver<Result<T, E>>,
}

impl<T: Send + 'static, E: Send + 'static> Future for WorkerFuture<T, E> {
    type Output = Result<T, E>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context) -> Poll<Self::Output> {
        let rec = unsafe { self.map_unchecked_mut(|s| &mut s.receiver) };
        match rec.poll(cx) {
            Poll::Ready(v) => {
                if let Ok(v) = v {
                    return Poll::Ready(v);
                } else {
                    panic!("Worker future can not have canceled sender");
                }
            }
            Poll::Pending => {
                return Poll::Pending;
            }
        }
    }
}

impl<T: Send + 'static, E: Send + 'static> WorkerFuture<T, E> {
    pub fn wait(self) -> <Self as Future>::Output {
        block_on(self)
    }
}

fn log2_floor(num: usize) -> u32 {
    assert!(num > 0);

    let mut pow = 0;

    while (1 << (pow + 1)) <= num {
        pow += 1;
    }

    pow
}

#[test]
fn test_log2_floor() {
    assert_eq!(log2_floor(1), 0);
    assert_eq!(log2_floor(2), 1);
    assert_eq!(log2_floor(3), 1);
    assert_eq!(log2_floor(4), 2);
    assert_eq!(log2_floor(5), 2);
    assert_eq!(log2_floor(6), 2);
    assert_eq!(log2_floor(7), 2);
    assert_eq!(log2_floor(8), 3);
}

#[test]
fn test_trivial_spawning() {
    use self::futures::executor::block_on;

    fn long_fn() -> Result<usize, ()> {
        let mut i: usize = 1;
        println!("Start calculating long task");
        for _ in 0..1000000 {
            i = i.wrapping_mul(42);
        }

        println!("Done calculating long task");

        Ok(i)
    }

    let worker = Worker::new();
    println!("Spawning");
    let fut = worker.compute(|| long_fn());
    println!("Done spawning");

    println!("Will sleep now");

    std::thread::sleep(std::time::Duration::from_millis(10000));

    println!("Done sleeping");

    let _ = block_on(fut);
}
