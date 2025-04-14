use crate::pairing::{CurveAffine, CurveProjective, Engine};

use crate::pairing::ff::{Field, PrimeField, PrimeFieldRepr, ScalarEngine};

use super::source::*;
use std::future::Future;
use std::pin::Pin;
use std::sync::Arc;
use std::task::{Context, Poll};

extern crate futures;

use self::futures::executor::block_on;
use self::futures::future::{join_all, JoinAll};

use super::worker::{Worker, WorkerFuture};

use super::SynthesisError;

use cfg_if;

/// This genious piece of code works in the following way:
/// - choose `c` - the bit length of the region that one thread works on
/// - make `2^c - 1` buckets and initialize them with `G = infinity` (that's equivalent of zero)
/// - there is no bucket for "zero" cause it's not necessary
/// - go over the pairs `(base, scalar)`
/// - for each scalar calculate `scalar % 2^c` and add the base (without any multiplications!) to the
/// corresponding bucket
/// - at the end each bucket will have an accumulated value that should be multiplied by the corresponding factor
/// between `1` and `2^c - 1` to get the right value
/// - here comes the first trick - you don't need to do multiplications at all, just add all the buckets together
/// starting from the first one `(a + b + c + ...)` and than add to the first sum another sum of the form
/// `(b + c + d + ...)`, and than the third one `(c + d + ...)`, that will result in the proper prefactor infront of every
/// accumulator, without any multiplication operations at all
/// - that's of course not enough, so spawn the next thread
/// - this thread works with the same bit width `c`, but SKIPS lowers bits completely, so it actually takes values
/// in the form `(scalar >> c) % 2^c`, so works on the next region
/// - spawn more threads until you exhaust all the bit length
/// - you will get roughly `[bitlength / c] + 1` inaccumulators
/// - double the highest accumulator enough times, add to the next one, double the result, add the next accumulator, continue
///
/// Demo why it works:
/// ```text
///     a * G + b * H = (a_2 * (2^c)^2 + a_1 * (2^c)^1 + a_0) * G + (b_2 * (2^c)^2 + b_1 * (2^c)^1 + b_0) * H
/// ```
/// - make buckets over `0` labeled coefficients
/// - make buckets over `1` labeled coefficients
/// - make buckets over `2` labeled coefficients
/// - accumulators over each set of buckets will have an implicit factor of `(2^c)^i`, so before summing thme up
/// "higher" accumulators must be doubled `c` times
///
fn multiexp_inner<Q, D, G, S>(
    pool: &Worker,
    bases: S,
    density_map: D,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    skip: u32,
    c: u32,
    handle_trivial: bool,
) -> WorkerFuture<<G as CurveAffine>::Projective, SynthesisError>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
    G: CurveAffine,
    S: SourceBuilder<G>,
{
    // Perform this region of the multiexp
    let this = {
        // let bases = bases.clone();
        // let exponents = exponents.clone();
        // let density_map = density_map.clone();

        // This is a Pippenger’s algorithm
        pool.compute(move || {
            // Accumulate the result
            let mut acc = G::Projective::zero();

            // Build a source for the bases
            let mut bases = bases.new();

            // Create buckets to place remainders s mod 2^c,
            // it will be 2^c - 1 buckets (no bucket for zeroes)

            // Create space for the buckets
            let mut buckets = vec![<G as CurveAffine>::Projective::zero(); (1 << c) - 1];

            let zero = <G::Engine as ScalarEngine>::Fr::zero().into_repr();
            let one = <G::Engine as ScalarEngine>::Fr::one().into_repr();

            // Sort the bases into buckets
            for (&exp, density) in exponents.iter().zip(density_map.as_ref().iter()) {
                // Go over density and exponents
                if density {
                    if exp == zero {
                        bases.skip(1)?;
                    } else if exp == one {
                        if handle_trivial {
                            bases.add_assign_mixed(&mut acc)?;
                        } else {
                            bases.skip(1)?;
                        }
                    } else {
                        // Place multiplication into the bucket: Separate s * P as
                        // (s/2^c) * P + (s mod 2^c) P
                        // First multiplication is c bits less, so one can do it,
                        // sum results from different buckets and double it c times,
                        // then add with (s mod 2^c) P parts
                        let mut exp = exp;
                        exp.shr(skip);
                        let exp = exp.as_ref()[0] % (1 << c);

                        if exp != 0 {
                            bases.add_assign_mixed(&mut buckets[(exp - 1) as usize])?;
                        } else {
                            bases.skip(1)?;
                        }
                    }
                }
            }

            // also measure this cycle:
            let start = std::time::Instant::now();

            // Summation by parts
            // e.g. 3a + 2b + 1c = a +
            //                    (a) + b +
            //                    ((a) + b) + c
            let mut running_sum = G::Projective::zero();
            for exp in buckets.into_iter().rev() {
                running_sum.add_assign(&exp);
                acc.add_assign(&running_sum);
            }

            if skip == 0 {
                let duration_ns = start.elapsed().as_nanos() as f64;
                println!("Elapsed {} ns for special loop", duration_ns);
            }

            Ok(acc)
        })
    };

    this
}

#[inline(always)]
fn multiexp_inner_impl<Q, D, G, S>(
    pool: &Worker,
    bases: S,
    density_map: D,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    skip: u32,
    c: u32,
    handle_trivial: bool,
) -> WorkerFuture<<G as CurveAffine>::Projective, SynthesisError>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
    G: CurveAffine,
    S: SourceBuilder<G>,
{
    multiexp_inner(pool, bases, density_map, exponents, skip, c, handle_trivial)
    // multiexp_inner_with_prefetch_stable(pool, bases, density_map, exponents, skip, c, handle_trivial)
}

fn multiexp_inner_with_prefetch_stable<Q, D, G, S>(
    pool: &Worker,
    bases: S,
    density_map: D,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    skip: u32,
    c: u32,
    handle_trivial: bool,
) -> WorkerFuture<<G as CurveAffine>::Projective, SynthesisError>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
    G: CurveAffine,
    S: SourceBuilder<G>,
{
    // Perform this region of the multiexp
    let this = {
        let bases = bases.clone();
        let exponents = exponents.clone();
        let density_map = density_map.clone();

        // This is a Pippenger’s algorithm
        pool.compute(move || {
            // Accumulate the result
            let mut acc = G::Projective::zero();

            // Build a source for the bases
            let mut bases = bases.new();

            // Create buckets to place remainders s mod 2^c,
            // it will be 2^c - 1 buckets (no bucket for zeroes)

            // Create space for the buckets
            let mut buckets = vec![<G as CurveAffine>::Projective::zero(); (1 << c) - 1];

            let zero = <G::Engine as ScalarEngine>::Fr::zero().into_repr();
            let one = <G::Engine as ScalarEngine>::Fr::one().into_repr();
            let padding = Arc::new(vec![zero]);

            let mask = (1u64 << c) - 1;

            // Sort the bases into buckets
            for ((&exp, &next_exp), density) in exponents.iter().zip(exponents.iter().skip(1).chain(padding.iter())).zip(density_map.as_ref().iter()) {
                // no matter what happens - prefetch next bucket
                if next_exp != zero && next_exp != one {
                    let mut next_exp = next_exp;
                    next_exp.shr(skip);
                    let next_exp = next_exp.as_ref()[0] & mask;
                    if next_exp != 0 {
                        let p: *const <G as CurveAffine>::Projective = &buckets[(next_exp - 1) as usize];
                        crate::prefetch::prefetch_l1_pointer(p);
                    }
                }
                // Go over density and exponents
                if density {
                    if exp == zero {
                        bases.skip(1)?;
                    } else if exp == one {
                        if handle_trivial {
                            bases.add_assign_mixed(&mut acc)?;
                        } else {
                            bases.skip(1)?;
                        }
                    } else {
                        // Place multiplication into the bucket: Separate s * P as
                        // (s/2^c) * P + (s mod 2^c) P
                        // First multiplication is c bits less, so one can do it,
                        // sum results from different buckets and double it c times,
                        // then add with (s mod 2^c) P parts
                        let mut exp = exp;
                        exp.shr(skip);
                        let exp = exp.as_ref()[0] % mask;

                        if exp != 0 {
                            bases.add_assign_mixed(&mut buckets[(exp - 1) as usize])?;
                        } else {
                            bases.skip(1)?;
                        }
                    }
                }
            }

            // Summation by parts
            // e.g. 3a + 2b + 1c = a +
            //                    (a) + b +
            //                    ((a) + b) + c
            let mut running_sum = G::Projective::zero();
            for exp in buckets.into_iter().rev() {
                running_sum.add_assign(&exp);
                acc.add_assign(&running_sum);
            }

            Ok(acc)
        })
    };

    this
}

/// Perform multi-exponentiation. The caller is responsible for ensuring the
/// query size is the same as the number of exponents.
pub fn future_based_multiexp<G: CurveAffine>(pool: &Worker, bases: Arc<Vec<G>>, exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>) -> ChunksJoiner<<G as CurveAffine>::Projective> {
    assert!(exponents.len() <= bases.len());
    let c = if exponents.len() < 32 {
        3u32
    } else {
        let mut width = (f64::from(exponents.len() as u32)).ln().ceil() as u32;
        let mut num_chunks = <G::Scalar as PrimeField>::NUM_BITS / width;
        if <G::Scalar as PrimeField>::NUM_BITS % width != 0 {
            num_chunks += 1;
        }

        if num_chunks < pool.cpus as u32 {
            width = <G::Scalar as PrimeField>::NUM_BITS / (pool.cpus as u32);
            if <G::Scalar as PrimeField>::NUM_BITS % (pool.cpus as u32) != 0 {
                width += 1;
            }
        }

        width
    };

    let mut skip = 0;
    let mut futures = Vec::with_capacity((<G::Engine as ScalarEngine>::Fr::NUM_BITS / c + 1) as usize);

    while skip < <G::Engine as ScalarEngine>::Fr::NUM_BITS {
        let chunk_future = if skip == 0 {
            future_based_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), 0, c, true)
        } else {
            future_based_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), skip, c, false)
        };

        futures.push(chunk_future);
        skip += c;
    }

    let join = join_all(futures);

    ChunksJoiner { join, c }
}

/// Perform multi-exponentiation. The caller is responsible for ensuring the
/// query size is the same as the number of exponents.
pub fn future_based_dense_multiexp_over_fixed_width_windows<G: CurveAffine>(
    pool: &Worker,
    bases: Arc<Vec<G>>,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    c: u32,
) -> ChunksJoiner<<G as CurveAffine>::Projective> {
    assert!(exponents.len() <= bases.len());

    let mut skip = 0;
    let mut futures = Vec::with_capacity((<G::Engine as ScalarEngine>::Fr::NUM_BITS / c + 1) as usize);

    while skip < <G::Engine as ScalarEngine>::Fr::NUM_BITS {
        let chunk_future = if skip == 0 {
            // future_based_buffered_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), 0, c, true)
            future_based_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), 0, c, true)
        } else {
            // future_based_buffered_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), skip, c, false)
            future_based_dense_multiexp_impl(pool, bases.clone(), exponents.clone(), skip, c, false)
        };

        futures.push(chunk_future);
        skip += c;
    }

    let join = join_all(futures);

    ChunksJoiner { join, c }
}

fn future_based_dense_multiexp_impl<G: CurveAffine>(
    pool: &Worker,
    bases: Arc<Vec<G>>,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    skip: u32,
    c: u32,
    handle_trivial: bool,
) -> WorkerFuture<<G as CurveAffine>::Projective, SynthesisError> {
    // Perform this region of the multiexp
    let this = {
        let bases = bases.clone();
        let exponents = exponents.clone();
        let bases = bases.clone();

        // This is a Pippenger’s algorithm
        pool.compute(move || {
            // Accumulate the result
            let mut acc = G::Projective::zero();

            // Create buckets to place remainders s mod 2^c,
            // it will be 2^c - 1 buckets (no bucket for zeroes)

            // Create space for the buckets
            let mut buckets = vec![<G as CurveAffine>::Projective::zero(); (1 << c) - 1];

            let zero = <G::Engine as ScalarEngine>::Fr::zero().into_repr();
            let one = <G::Engine as ScalarEngine>::Fr::one().into_repr();
            let padding = Arc::new(vec![zero]);

            let mask = 1 << c;

            // Sort the bases into buckets
            for ((&exp, base), &next_exp) in exponents.iter().zip(bases.iter()).zip(exponents.iter().skip(1).chain(padding.iter())) {
                // no matter what happens - prefetch next bucket
                if next_exp != zero && next_exp != one {
                    let mut next_exp = next_exp;
                    next_exp.shr(skip);
                    let next_exp = next_exp.as_ref()[0] % mask;
                    if next_exp != 0 {
                        let p: *const <G as CurveAffine>::Projective = &buckets[(next_exp - 1) as usize];
                        crate::prefetch::prefetch_l1_pointer(p);
                    }
                }
                // Go over density and exponents
                if exp == zero {
                    continue;
                } else if exp == one {
                    if handle_trivial {
                        acc.add_assign_mixed(base);
                    } else {
                        continue;
                    }
                } else {
                    // Place multiplication into the bucket: Separate s * P as
                    // (s/2^c) * P + (s mod 2^c) P
                    // First multiplication is c bits less, so one can do it,
                    // sum results from different buckets and double it c times,
                    // then add with (s mod 2^c) P parts
                    let mut exp = exp;
                    exp.shr(skip);
                    let exp = exp.as_ref()[0] % mask;

                    if exp != 0 {
                        (&mut buckets[(exp - 1) as usize]).add_assign_mixed(base);
                    } else {
                        continue;
                    }
                }
            }

            // Summation by parts
            // e.g. 3a + 2b + 1c = a +
            //                    (a) + b +
            //                    ((a) + b) + c
            let mut running_sum = G::Projective::zero();
            for exp in buckets.into_iter().rev() {
                running_sum.add_assign(&exp);
                acc.add_assign(&running_sum);
            }

            Ok(acc)
        })
    };

    this
}

fn future_based_buffered_dense_multiexp_impl<G: CurveAffine>(
    pool: &Worker,
    bases: Arc<Vec<G>>,
    exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>,
    skip: u32,
    c: u32,
    handle_trivial: bool,
) -> WorkerFuture<<G as CurveAffine>::Projective, SynthesisError> {
    // Perform this region of the multiexp
    let this = {
        let bases = bases.clone();
        let exponents = exponents.clone();
        let bases = bases.clone();

        // This is a Pippenger’s algorithm
        pool.compute(move || {
            // Accumulate the result
            let mut acc = G::Projective::zero();

            // Create buckets to place remainders s mod 2^c,
            // it will be 2^c - 1 buckets (no bucket for zeroes)

            // Create space for the buckets
            let mut buckets = vec![<G as CurveAffine>::Projective::zero(); (1 << c) - 1];

            let zero = <G::Engine as ScalarEngine>::Fr::zero().into_repr();
            let one = <G::Engine as ScalarEngine>::Fr::one().into_repr();

            let mask = 1 << c;

            const BUFFER_SIZE: usize = 64;
            let mut buffers: Vec<Vec<G>> = vec![Vec::with_capacity(BUFFER_SIZE); (1 << c) - 1];

            // Sort the bases into buckets
            for (&exp, &base) in exponents.iter().zip(bases.iter()) {
                // Go over density and exponents
                if exp == zero {
                    continue;
                } else if exp == one {
                    if handle_trivial {
                        acc.add_assign_mixed(&base);
                    } else {
                        continue;
                    }
                } else {
                    // Place multiplication into the bucket: Separate s * P as
                    // (s/2^c) * P + (s mod 2^c) P
                    // First multiplication is c bits less, so one can do it,
                    // sum results from different buckets and double it c times,
                    // then add with (s mod 2^c) P parts
                    let mut exp = exp;
                    exp.shr(skip);
                    let exp = exp.as_ref()[0] % mask;

                    if exp != 0 {
                        let idx = (exp - 1) as usize;
                        if buffers[idx].len() == BUFFER_SIZE {
                            let mut el = buckets[idx];
                            for b in buffers[idx].iter() {
                                el.add_assign_mixed(&b);
                            }
                            buffers[idx].truncate(0);
                            buckets[idx] = el;
                        }

                        buffers[idx].push(base);
                    } else {
                        continue;
                    }
                }
            }

            // we have some unprocessed left, so add them to the buckets
            for (idx, buffer) in buffers.into_iter().enumerate() {
                let mut el = buckets[idx];
                for b in buffer.into_iter() {
                    el.add_assign_mixed(&b);
                }
                buckets[idx] = el;
            }

            // Summation by parts
            // e.g. 3a + 2b + 1c = a +
            //                    (a) + b +
            //                    ((a) + b) + c
            let mut running_sum = G::Projective::zero();
            for exp in buckets.into_iter().rev() {
                running_sum.add_assign(&exp);
                acc.add_assign(&running_sum);
            }

            Ok(acc)
        })
    };

    this
}

/// Perform multi-exponentiation. The caller is responsible for ensuring the
/// query size is the same as the number of exponents.
pub fn multiexp<Q, D, G, S>(pool: &Worker, bases: S, density_map: D, exponents: Arc<Vec<<<G::Engine as ScalarEngine>::Fr as PrimeField>::Repr>>) -> ChunksJoiner<<G as CurveAffine>::Projective>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
    G: CurveAffine,
    S: SourceBuilder<G>,
{
    let c = if exponents.len() < 32 { 3u32 } else { (f64::from(exponents.len() as u32)).ln().ceil() as u32 };

    if let Some(query_size) = density_map.as_ref().get_query_size() {
        // If the density map has a known query size, it should not be
        // inconsistent with the number of exponents.

        assert!(query_size == exponents.len());
    }

    let mut skip = 0;
    let mut futures = Vec::with_capacity((<G::Engine as ScalarEngine>::Fr::NUM_BITS / c + 1) as usize);

    while skip < <G::Engine as ScalarEngine>::Fr::NUM_BITS {
        let chunk_future = if skip == 0 {
            multiexp_inner_impl(pool, bases.clone(), density_map.clone(), exponents.clone(), 0, c, true)
        } else {
            multiexp_inner_impl(pool, bases.clone(), density_map.clone(), exponents.clone(), skip, c, false)
        };

        futures.push(chunk_future);
        skip += c;
    }

    let join = join_all(futures);

    ChunksJoiner { join, c }
}

pub(crate) fn multiexp_with_fixed_width<Q, D, G, S>(
    pool: &Worker,
    bases: S,
    density_map: D,
    exponents: Arc<Vec<<<G::Engine as ScalarEngine>::Fr as PrimeField>::Repr>>,
    c: u32,
) -> ChunksJoiner<<G as CurveAffine>::Projective>
where
    for<'a> &'a Q: QueryDensity,
    D: Send + Sync + 'static + Clone + AsRef<Q>,
    G: CurveAffine,
    S: SourceBuilder<G>,
{
    if let Some(query_size) = density_map.as_ref().get_query_size() {
        // If the density map has a known query size, it should not be
        // inconsistent with the number of exponents.

        assert!(query_size == exponents.len());
    }

    let mut skip = 0;
    let mut futures = Vec::with_capacity((<G::Engine as ScalarEngine>::Fr::NUM_BITS / c + 1) as usize);

    while skip < <G::Engine as ScalarEngine>::Fr::NUM_BITS {
        let chunk_future = if skip == 0 {
            multiexp_inner_impl(pool, bases.clone(), density_map.clone(), exponents.clone(), 0, c, true)
        } else {
            multiexp_inner_impl(pool, bases.clone(), density_map.clone(), exponents.clone(), skip, c, false)
        };

        futures.push(chunk_future);
        skip += c;
    }

    let join = join_all(futures);

    ChunksJoiner { join, c }
}

pub struct ChunksJoiner<G: CurveProjective> {
    join: JoinAll<WorkerFuture<G, SynthesisError>>,
    c: u32,
}

impl<G: CurveProjective> Future for ChunksJoiner<G> {
    type Output = Result<G, SynthesisError>;

    fn poll(self: Pin<&mut Self>, cx: &mut Context) -> Poll<Self::Output> {
        let c = self.as_ref().c;
        let join = unsafe { self.map_unchecked_mut(|s| &mut s.join) };
        match join.poll(cx) {
            Poll::Ready(v) => {
                let v = join_chunks(v, c);
                return Poll::Ready(v);
            }
            Poll::Pending => {
                return Poll::Pending;
            }
        }
    }
}

impl<G: CurveProjective> ChunksJoiner<G> {
    pub fn wait(self) -> <Self as Future>::Output {
        block_on(self)
    }
}

fn join_chunks<G: CurveProjective>(chunks: Vec<Result<G, SynthesisError>>, c: u32) -> Result<G, SynthesisError> {
    if chunks.len() == 0 {
        return Ok(G::zero());
    }

    let mut iter = chunks.into_iter().rev();
    let higher = iter.next().expect("is some chunk result");
    let mut higher = higher?;

    for chunk in iter {
        let this = chunk?;
        for _ in 0..c {
            higher.double();
        }

        higher.add_assign(&this);
    }

    Ok(higher)
}

/// Perform multi-exponentiation. The caller is responsible for ensuring that
/// the number of bases is the same as the number of exponents.
#[allow(dead_code)]
pub fn dense_multiexp<G: CurveAffine>(pool: &Worker, bases: &[G], exponents: &[<<G::Engine as ScalarEngine>::Fr as PrimeField>::Repr]) -> Result<<G as CurveAffine>::Projective, SynthesisError> {
    if exponents.len() != bases.len() {
        return Err(SynthesisError::AssignmentMissing);
    }
    // do some heuristics here
    // we proceed chunks of all points, and all workers do the same work over
    // some scalar width, so to have expected number of additions into buckets to 1
    // we have to take log2 from the expected chunk(!) length
    let c = if exponents.len() < 32 {
        3u32
    } else {
        let chunk_len = pool.get_chunk_size(exponents.len());
        (f64::from(chunk_len as u32)).ln().ceil() as u32

        // (f64::from(exponents.len() as u32)).ln().ceil() as u32
    };

    // dense_multiexp_inner_unrolled_with_prefetch(pool, bases, exponents, 0, c, true)
    dense_multiexp_inner(pool, bases, exponents, 0, c, true)
}

fn dense_multiexp_inner<G: CurveAffine>(
    pool: &Worker,
    bases: &[G],
    exponents: &[<<G::Engine as ScalarEngine>::Fr as PrimeField>::Repr],
    mut skip: u32,
    c: u32,
    handle_trivial: bool,
) -> Result<<G as CurveAffine>::Projective, SynthesisError> {
    use std::sync::Mutex;
    // Perform this region of the multiexp. We use a different strategy - go over region in parallel,
    // then over another region, etc. No Arc required
    let this = {
        // let mask = (1u64 << c) - 1u64;
        let this_region = Mutex::new(<G as CurveAffine>::Projective::zero());
        let arc = Arc::new(this_region);
        pool.scope(bases.len(), |scope, chunk| {
            for (base, exp) in bases.chunks(chunk).zip(exponents.chunks(chunk)) {
                let this_region_rwlock = arc.clone();
                // let handle =
                scope.spawn(move |_| {
                    let mut buckets = vec![<G as CurveAffine>::Projective::zero(); (1 << c) - 1];
                    // Accumulate the result
                    let mut acc = G::Projective::zero();
                    let zero = <G::Engine as ScalarEngine>::Fr::zero().into_repr();
                    let one = <G::Engine as ScalarEngine>::Fr::one().into_repr();

                    for (base, &exp) in base.iter().zip(exp.iter()) {
                        // let index = (exp.as_ref()[0] & mask) as usize;

                        // if index != 0 {
                        //     buckets[index - 1].add_assign_mixed(base);
                        // }

                        // exp.shr(c as u32);

                        if exp != zero {
                            if exp == one {
                                if handle_trivial {
                                    acc.add_assign_mixed(base);
                                }
                            } else {
                                let mut exp = exp;
                                exp.shr(skip);
                                let exp = exp.as_ref()[0] % (1 << c);
                                if exp != 0 {
                                    buckets[(exp - 1) as usize].add_assign_mixed(base);
                                }
                            }
                        }
                    }

                    // buckets are filled with the corresponding accumulated value, now sum
                    let mut running_sum = G::Projective::zero();
                    for exp in buckets.into_iter().rev() {
                        running_sum.add_assign(&exp);
                        acc.add_assign(&running_sum);
                    }

                    let mut guard = match this_region_rwlock.lock() {
                        Ok(guard) => guard,
                        Err(_) => {
                            panic!("poisoned!");
                            // poisoned.into_inner()
                        }
                    };

                    (*guard).add_assign(&acc);
                });
            }
        });

        let this_region = Arc::try_unwrap(arc).unwrap();
        let this_region = this_region.into_inner().unwrap();

        this_region
    };

    skip += c;

    if skip >= <G::Engine as ScalarEngine>::Fr::NUM_BITS {
        // There isn't another region, and this will be the highest region
        return Ok(this);
    } else {
        // next region is actually higher than this one, so double it enough times
        let mut next_region = dense_multiexp_inner(pool, bases, exponents, skip, c, false).unwrap();
        for _ in 0..c {
            next_region.double();
        }

        next_region.add_assign(&this);

        return Ok(next_region);
    }
}

fn get_window_size_for_length(length: usize, chunk_length: usize) -> u32 {
    if length < 32 {
        return 3u32;
    } else {
        let exact = (f64::from(chunk_length as u32)).ln();
        let floor = exact.floor();
        if exact > floor + 0.5f64 {
            return exact.ceil() as u32;
        } else {
            return floor as u32;
        }

        // (f64::from(chunk_length as u32)).ln().ceil() as u32
        // (f64::from(length as u32)).ln().ceil() as u32
    };
}

#[cfg(test)]
mod test {
    use super::*;

    fn naive_multiexp<G: CurveAffine>(bases: Arc<Vec<G>>, exponents: Arc<Vec<<G::Scalar as PrimeField>::Repr>>) -> G::Projective {
        assert_eq!(bases.len(), exponents.len());

        let mut acc = G::Projective::zero();

        for (base, exp) in bases.iter().zip(exponents.iter()) {
            acc.add_assign(&base.mul(*exp));
        }

        acc
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_new_multiexp_with_bls12() {
        use crate::pairing::bls12_381::Bls12;
        use rand::{self, Rand};

        use self::futures::executor::block_on;

        const SAMPLES: usize = 1 << 14;

        let rng = &mut rand::thread_rng();
        let v = Arc::new((0..SAMPLES).map(|_| <Bls12 as ScalarEngine>::Fr::rand(rng).into_repr()).collect::<Vec<_>>());
        let g = Arc::new((0..SAMPLES).map(|_| <Bls12 as Engine>::G1::rand(rng).into_affine()).collect::<Vec<_>>());

        let naive = naive_multiexp(g.clone(), v.clone());

        let pool = Worker::new();

        let fast = block_on(multiexp(&pool, (g, 0), FullDensity, v)).unwrap();

        assert_eq!(naive, fast);
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn test_valid_bn254_multiexp() {
        use crate::pairing::bn256::Bn256;
        use rand::{self, Rand};

        const SAMPLES: usize = 1 << 22;

        let pool = Worker::new();

        let rng = &mut rand::thread_rng();
        let v = (0..SAMPLES).map(|_| <Bn256 as ScalarEngine>::Fr::rand(rng).into_repr()).collect::<Vec<_>>();
        let g = (0..SAMPLES).map(|_| <Bn256 as Engine>::G1::rand(rng).into_affine()).collect::<Vec<_>>();
        let dense = dense_multiexp(&pool, &g, &v).unwrap();

        let v = Arc::new(v);
        let g = Arc::new(g);

        let naive = naive_multiexp(g.clone(), v.clone());

        assert_eq!(dense, naive);

        use self::futures::executor::block_on;

        let fast_dense = future_based_multiexp(&pool, g.clone(), v.clone()).wait().unwrap();

        assert_eq!(naive, fast_dense);

        let fast = block_on(multiexp(&pool, (g, 0), FullDensity, v)).unwrap();

        assert_eq!(naive, fast);
    }

    #[test]
    #[ignore]
    fn test_new_multexp_speed_with_bn256() {
        use crate::pairing::bn256::Bn256;
        use num_cpus;
        use rand::{self, Rand};

        let cpus = num_cpus::get();
        const SAMPLES: usize = 1 << 22;

        let rng = &mut rand::thread_rng();
        let v = Arc::new((0..SAMPLES).map(|_| <Bn256 as ScalarEngine>::Fr::rand(rng).into_repr()).collect::<Vec<_>>());
        let g = Arc::new((0..SAMPLES).map(|_| <Bn256 as Engine>::G1::rand(rng).into_affine()).collect::<Vec<_>>());

        let pool = Worker::new();

        use self::futures::executor::block_on;

        let start = std::time::Instant::now();

        let _fast = block_on(multiexp(&pool, (g, 0), FullDensity, v)).unwrap();

        let duration_ns = start.elapsed().as_nanos() as f64;
        println!("Elapsed {} ns for {} samples", duration_ns, SAMPLES);
        let time_per_sample = duration_ns / (SAMPLES as f64);
        println!("Tested on {} samples on {} CPUs with {} ns per multiplication", SAMPLES, cpus, time_per_sample);
    }

    fn calculate_parameters(size: usize, threads: usize, bits: u32) {
        let mut chunk_len = size / threads;
        if size / threads != 0 {
            chunk_len += 1;
        }
        let raw_size = (f64::from(chunk_len as u32)).ln();
        let new_window_size = if raw_size.floor() + 0.5 < raw_size { raw_size.ceil() as u32 } else { raw_size.floor() as u32 };
        let window_size = (f64::from(chunk_len as u32)).ln().ceil() as u32;

        let mut num_windows = bits / window_size;
        let leftover = bits % window_size;
        if leftover != 0 {
            num_windows += 1;
        }

        let uncompensated_window = (f64::from(size as u32)).ln().ceil() as u32;
        let mut num_uncompensated_windows = bits / uncompensated_window;
        let uncompensated_leftover = bits % uncompensated_window;
        if uncompensated_leftover != 0 {
            num_uncompensated_windows += 1;
        }

        println!(
            "For size {} and {} cores: chunk len {}, {} windows, average window {} bits, leftover {} bits. Alternative window size = {}",
            size, threads, chunk_len, num_windows, window_size, leftover, new_window_size
        );
        // println!("Raw window size = {}", raw_size);
        // println!("Uncompensated: {} windows, arevage window {} bits, leftover {} bits", num_uncompensated_windows, uncompensated_window, uncompensated_leftover);

        // (f64::from(exponents.len() as u32)).ln().ceil() as u32
    }

    #[test]
    fn test_sizes_for_bn254() {
        let sizes = vec![1 << 23, 1 << 24];
        let cores = vec![8, 12, 16, 24, 32, 48];
        for size in sizes {
            for &core in &cores {
                calculate_parameters(size, core, 254);
            }
        }
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn bench_bls_addition() {
        use crate::pairing::bls12_381::Bls12;
        use rand::{self, Rand};

        let size = 100000u32;
        let rng = &mut rand::thread_rng();

        let A = (0..size).map(|_| <Bls12 as Engine>::G1::rand(rng)).collect::<Vec<_>>();
        let B = (0..size).map(|_| <Bls12 as Engine>::G1::rand(rng)).collect::<Vec<_>>();

        let start = std::time::Instant::now();

        let C = (0..size)
            .map(|i| {
                let mut temp = A[i as usize];
                temp.add_assign(&B[i as usize]);
                temp
            })
            .collect::<Vec<_>>();

        let duration_ns = start.elapsed().as_nanos() as f64;
        println!("Elapsed {} ns for {} samples", duration_ns, size);
        let time_per_sample = duration_ns / (size as f64);
        println!("Elapsed {} ns per sample", time_per_sample);
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn bench_bls_doubling() {
        use crate::pairing::bls12_381::Bls12;
        use rand::{self, Rand};

        let size = 100000u32;
        let rng = &mut rand::thread_rng();

        let A = (0..size).map(|_| <Bls12 as Engine>::G1::rand(rng)).collect::<Vec<_>>();

        let start = std::time::Instant::now();

        let B = (0..size)
            .map(|i| {
                let mut temp = A[i as usize];
                temp.double();
                temp
            })
            .collect::<Vec<_>>();

        let duration_ns = start.elapsed().as_nanos() as f64;
        println!("Elapsed {} ns for {} samples", duration_ns, size);
        let time_per_sample = duration_ns / (size as f64);
        println!("Elapsed {} ns per sample", time_per_sample);
    }

    #[test]
    #[ignore] // TODO(ignored-test): Timeout.
    fn bench_Pippenger_with_small_chunk() {
        use crate::pairing::bls12_381::Bls12;
        use rand::{self, Rand};

        let size = 1000000u32;
        let rng = &mut rand::thread_rng();

        let v = Arc::new((0..size).map(|_| <Bls12 as ScalarEngine>::Fr::rand(rng).into_repr()).collect::<Vec<_>>());
        let g = Arc::new((0..size).map(|_| <Bls12 as Engine>::G1::rand(rng).into_affine()).collect::<Vec<_>>());

        let pool = Worker::new();
        println!("loading {} cpus", pool.cpus);

        let start = std::time::Instant::now();

        let fast = block_on(multiexp(&pool, (g, 0), FullDensity, v)).unwrap();

        let duration_ns = start.elapsed().as_nanos() as f64;
        println!("Elapsed {} ns for Pippenger", duration_ns);
        let time_per_sample = duration_ns / (size as f64);
        println!("Elapsed {} ns per sample", time_per_sample);
    }
}
