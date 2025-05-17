//! This is a dummy interface to substitute multicore worker
//! in environments like WASM
extern crate futures;

use std::marker::PhantomData;

use std::future::Future;
use std::pin::Pin;
use std::task::{Context, Poll};

use self::futures::channel::oneshot::{channel, Receiver, Sender};
use self::futures::executor::block_on;

#[derive(Clone)]
pub struct Worker {
    pub(crate) cpus: usize,
}

impl Worker {
    // We don't expose this outside the library so that
    // all `Worker` instances have the same number of
    // CPUs configured.
    pub(crate) fn new_with_cpus(_cpus: usize) -> Worker {
        Worker { cpus: 1 }
    }

    pub fn new() -> Worker {
        Self::new_with_cpus(1)
    }

    pub fn log_num_cpus(&self) -> u32 {
        0u32
    }

    pub fn compute<F, T, E>(&self, f: F) -> WorkerFuture<T, E>
    where
        F: FnOnce() -> Result<T, E> + Send + 'static,
        T: Send + 'static,
        E: Send + 'static,
    {
        let result = f();

        let (sender, receiver) = channel();
        let _ = sender.send(result);

        let worker_future = WorkerFuture { receiver };

        worker_future
    }

    pub fn scope<'a, F, R>(&self, elements: usize, f: F) -> R
    where
        F: FnOnce(&Scope<'a>, usize) -> R,
    {
        let chunk_size = if elements == 0 { 1 } else { elements };

        let scope = Scope { _marker: PhantomData };

        f(&scope, chunk_size)
    }

    pub fn get_chunk_size(&self, elements: usize) -> usize {
        elements
    }

    pub fn get_num_spawned_threads(&self, elements: usize) -> usize {
        1
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
#[derive(Clone)]
pub struct Scope<'a> {
    _marker: PhantomData<&'a usize>,
}

impl<'a> Scope<'a> {
    pub fn spawn<F, R>(&self, f: F) -> R
    where
        F: FnOnce(&Scope<'a>) -> R,
    {
        f(&self)
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

#[test]
fn test_trivial_singlecore_spawning() {
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
