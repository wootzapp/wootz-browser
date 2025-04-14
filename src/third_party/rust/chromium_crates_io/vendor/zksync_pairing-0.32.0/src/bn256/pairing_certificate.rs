use crate::bn256::fq::*;
use crate::bn256::*;
use crate::{
    bn256::{G1Affine, G2Affine},
    CurveAffine, CurveProjective,
};
use ff::Field;

use super::{Fq12, Fq2};

pub fn prepare_all_line_functions(q: G2Affine) -> Vec<(Fq2, Fq2)> {
    fn line_double(t: G2Affine) -> (Fq2, Fq2) {
        let mut alpha = t.x;
        let mut mu = t.y;

        // alpha = 3 * x^2 / 2 * y
        alpha.square();
        let mut tmp = alpha.clone();
        tmp.double();
        alpha.add_assign(&tmp);

        let mut tmp = t.y;
        tmp.double();
        let tmp_inv = tmp.inverse().unwrap();
        alpha.mul_assign(&tmp_inv);

        let mut tmp = t.x;
        tmp.mul_assign(&alpha);
        mu.sub_assign(&tmp);

        (alpha, mu)
    }

    fn line_add(t: G2Affine, p: G2Affine) -> (Fq2, Fq2) {
        let x1 = t.x;
        let y1 = t.y;
        let x2 = p.x;
        let y2 = p.y;

        let mut alpha = y2;
        let mut mu = y1;

        // alpha = (y2 - y1) / (x2 - x1)
        alpha.sub_assign(&y1);
        let mut tmp = x2;
        tmp.sub_assign(&x1);
        let tmp_inv = tmp.inverse().unwrap();
        alpha.mul_assign(&tmp_inv);

        let mut tmp = x1;
        tmp.mul_assign(&alpha);
        mu.sub_assign(&tmp);

        (alpha, mu)
    }

    let mut l = vec![];
    let mut t = q.into_projective();
    let mut q_negated = q.clone();
    q_negated.negate();

    for i in (1..SIX_U_PLUS_2_NAF.len()).rev() {
        let (alpha, mu) = line_double(t.into_affine());
        t.double_with_alpha(alpha);
        l.push((alpha, mu));

        let bit = SIX_U_PLUS_2_NAF[i - 1];

        if bit != 0 {
            let q_t = if bit == 1 { q } else { q_negated };
            let (alpha, mu) = line_add(t.into_affine(), q_t);
            t.add_assign_mixed_with_alpha(&q_t, alpha);
            l.push((alpha, mu));
        }
    }

    // Frobenius map calculations for BN256
    let mut pi_1_q_x = q.x;
    let mut pi_1_q_y = q.y;
    pi_1_q_x.conjugate();
    pi_1_q_x.mul_assign(&FROBENIUS_COEFF_FQ6_C1[1]);
    pi_1_q_y.conjugate();
    pi_1_q_y.mul_assign(&XI_TO_Q_MINUS_1_OVER_2);
    let pi_1_q = G2Affine::from_xy_checked(pi_1_q_x, pi_1_q_y).unwrap();

    let mut pi_2_q_x = q.x;
    pi_2_q_x.mul_assign(&FROBENIUS_COEFF_FQ6_C1[2]);
    let pi_2_q = G2Affine::from_xy_checked(pi_2_q_x, q.y).unwrap();

    let (alpha, mu) = line_add(t.into_affine(), pi_1_q);
    t.add_assign_mixed_with_alpha(&pi_1_q, alpha);
    l.push((alpha.into(), mu.into()));

    let (alpha, mu) = line_add(t.into_affine(), pi_2_q);
    t.add_assign_mixed_with_alpha(&pi_2_q, alpha);
    l.push((alpha.into(), mu.into()));

    l
}
pub fn prepare_g1_point(p: G1Affine) -> G1Affine {
    // we "prepare" p by recomputing (x, y) -> (x', y') where: x' = - p.x / p.y; y' = -1 /p.y
    // it is required to enforce that in c0c3c4, c0 = 1
    let mut y_new = p.y.inverse().unwrap();
    y_new.negate();
    let mut x_new = p.x;
    x_new.mul_assign(&y_new);

    G1Affine::from_xy_unchecked(x_new, y_new)
}

pub fn miller_loop_with_prepared_lines(eval_points: &[G1Affine], lines: &[Vec<(Fq2, Fq2)>]) -> Fq12 {
    assert_eq!(eval_points.len(), lines.len());

    fn line_evaluation(alpha: &Fq2, mu: &Fq2, p: &G1Affine) -> (Fq2, Fq2, Fq2) {
        // previously: c0 = p.y; c3 = - lambda * p.x; c4 = -mu;
        // now we have: p = (x', y') where: x' = - p.x / p.y; y' = -1 /p.y
        // and compute c0 = 1, c3 = - lambda * p.x / p.y = lambda * p.x, c4 = - mu / p.y = mu * p.y

        // previously:
        // let mut c3 = *alpha;
        // c3.negate();
        // c3.c0.mul_assign(&p.x);
        // c3.c1.mul_assign(&p.x);
        // let mut c4 = *mu;
        // c4.negate();

        let mut c3 = *alpha;
        c3.c0.mul_assign(&p.x);
        c3.c1.mul_assign(&p.x);
        let mut c4 = *mu;
        c4.c0.mul_assign(&p.y);
        c4.c1.mul_assign(&p.y);
        let c0 = Fq2::one();

        (c0, c3, c4)
    }

    let mut f = Fq12::one();

    let mut lc = 0;
    for i in (1..SIX_U_PLUS_2_NAF.len()).rev() {
        if i != SIX_U_PLUS_2_NAF.len() - 1 {
            f.square();
        }
        let x = SIX_U_PLUS_2_NAF[i - 1];
        for (p, l) in eval_points.iter().zip(lines.iter()) {
            let (alpha, mu) = l[lc];
            let (c0, c1, c2) = line_evaluation(&alpha, &mu, p);
            f.mul_by_034(&c0, &c1, &c2);

            if x * x == 1 {
                let (alpha, bias) = l[lc + 1];
                let (c0, c1, c2) = line_evaluation(&alpha, &bias, p);
                f.mul_by_034(&c0, &c1, &c2);
            }
        }

        if x == 0 {
            lc += 1;
        } else {
            lc += 2;
        }
    }

    // Frobenius map part: p - p^2 + p^3 steps
    // this part runs through each eval point and applies
    // three additional line evaluations with special conditions.
    for (p, l) in eval_points.iter().zip(lines.iter()) {
        for k in 0..2 {
            let (alpha, mu) = l[lc + k];

            let (c0, c1, c2) = line_evaluation(&alpha, &mu, p);
            f.mul_by_034(&c0, &c1, &c2);
        }
    }

    lc += 2;

    // Check we consumed all lines
    if !lines.is_empty() {
        assert_eq!(lc, lines[0].len());
    }

    f
}

#[cfg(test)]
mod tests {
    use rand::Rand;

    use crate::{
        bn256::{miller_loop_with_prepared_lines, prepare_all_line_functions, prepare_g1_point, Bn256, G1Affine, G2Affine},
        compact_bn256, CurveAffine, Engine,
    };

    #[test]
    fn test_bn_equivalence() {
        let mut rng = rand::thread_rng();
        let g1 = G1Affine::rand(&mut rng);
        let g2 = G2Affine::rand(&mut rng);
        let miller_loop_result = Bn256::miller_loop(&[(&g1.prepare(), &g2.prepare())]);
        let lhs = Bn256::final_exponentiation(&miller_loop_result).unwrap();

        let g1 = compact_bn256::G1Affine::from_xy_checked(g1.x, g1.y).unwrap();
        let g2 = compact_bn256::G2Affine::from_xy_checked(g2.x, g2.y).unwrap();
        let miller_loop_result = compact_bn256::Bn256::miller_loop(&[(&g1.prepare(), &g2.prepare())]);
        let rhs = compact_bn256::Bn256::final_exponentiation(&miller_loop_result).unwrap();

        assert_eq!(lhs, rhs)
    }

    #[test]
    fn test_precomputed_lines() {
        let thread_rng = rand::thread_rng();
        let mut rng = thread_rng;
        let p = G1Affine::rand(&mut rng);
        let q = G2Affine::rand(&mut rng);

        let res = Bn256::miller_loop(&[(&p.prepare(), &q.prepare())]);
        let lhs = Bn256::final_exponentiation(&res);

        let lines = prepare_all_line_functions(q);
        let p_prepared = prepare_g1_point(p);

        let miller_loop_f = miller_loop_with_prepared_lines(&[p_prepared], &[lines]);
        let rhs = Bn256::final_exponentiation(&miller_loop_f);

        assert_eq!(lhs, rhs);
    }
}
