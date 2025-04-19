#[allow(unsafe_op_in_unsafe_fn)]
#[cxx::bridge]
mod ffi {
    // C++ types and signatures exposed to Rust
    extern "C++" {}

    // Rust types and signatures exposed to C++
    extern "Rust" {
        fn generate_groth16_proof(
            cert_hash: &[u8],
            headers_json: &str,
            content: &str,
            proving_key_bytes: &[u8]
        ) -> String;

        fn extract_public_inputs(
            cert_hash: &[u8],
            content: &str
        ) -> String;
        
        fn generate_keys(
            cert_hash: &[u8],
            headers_json: &str,
            content: &str
        ) -> String;
    }
}

use ark_bn254::{Bn254, Fr};
use ark_groth16::{Groth16, Proof, ProvingKey, VerifyingKey};
use ark_serialize::{CanonicalDeserialize, CanonicalSerialize};
use ark_relations::r1cs::{ConstraintSynthesizer, ConstraintSystemRef, SynthesisError};
use ark_snark::SNARK;
use ark_std::{Zero, One, io::Cursor};
use sha2::{Sha256, Digest};
use serde::{Serialize, Deserialize};
use ark_ff::PrimeField;
use ark_ec::pairing::Pairing;
use ark_std::rand::SeedableRng;

// Garaga-compatible JSON format
#[derive(Serialize, Deserialize)]
struct GaragaProof {
    pi_a: Vec<String>,
    pi_b: Vec<Vec<String>>,
    pi_c: Vec<String>,
    protocol: String,
    curve: String,
    public_signals: Vec<String>,
}

// Our circuit definition
#[derive(Clone)]
struct WebContentCircuit {
    // Inputs
    cert_hash: Vec<u8>,
    headers: String,
    content: String,
    
    // Public inputs
    cert_hash_public: Option<Fr>,
    content_hash_public: Option<Fr>,
}

impl ConstraintSynthesizer<Fr> for WebContentCircuit {
    fn generate_constraints(
        self,
        cs: ConstraintSystemRef<Fr>
    ) -> Result<(), SynthesisError> {
        // Create variables for the public inputs
        let _cert_hash_var = cs.new_input_variable(|| {
            self.cert_hash_public.ok_or(SynthesisError::AssignmentMissing)
        })?;
        
        let _content_hash_var = cs.new_input_variable(|| {
            self.content_hash_public.ok_or(SynthesisError::AssignmentMissing)
        })?;
        
        // Simple circuit - we're just verifying we know the inputs
        Ok(())
    }
}

// Helper function to convert bytes to field element
fn bytes_to_field(bytes: &[u8]) -> Fr {
    // Hash the bytes for uniformity
    let mut hasher = Sha256::new();
    hasher.update(bytes);
    let hash = hasher.finalize();
    
    // Convert hash to field element
    let mut value = Fr::zero();
    let bytes_to_use = std::cmp::min(hash.len(), 31); // Fr modulus is ~254 bits
    
    for i in 0..bytes_to_use {
        let byte = hash[i] as u64;
        let byte_fr = Fr::from(byte);
        
        // Multiply by 256^i to get the right position
        let mut position_multiplier = Fr::one();
        for _ in 0..i {
            position_multiplier *= Fr::from(256u64);
        }
        
        value += byte_fr * position_multiplier;
    }
    
    value
}

// Main function to generate proof
pub fn generate_groth16_proof(
    cert_hash: &[u8],
    headers_json: &str,
    content: &str,
    proving_key_bytes: &[u8]
) -> String {
    // Convert inputs to field elements
    let cert_hash_field = bytes_to_field(cert_hash);
    
    // Hash the content
    let mut content_hasher = Sha256::new();
    content_hasher.update(content.as_bytes());
    let content_hash = content_hasher.finalize();
    let content_hash_field = bytes_to_field(&content_hash);
    
    // Deserialize the proving key with the correct method
    let mut cursor = Cursor::new(proving_key_bytes);
    let proving_key = match ProvingKey::<Bn254>::deserialize_with_mode(&mut cursor, ark_serialize::Compress::No, ark_serialize::Validate::No) {
        Ok(pk) => pk,
        Err(_) => return String::from("Failed to deserialize proving key"),
    };
    
    // Create the circuit
    let circuit = WebContentCircuit {
        cert_hash: cert_hash.to_vec(),
        headers: headers_json.to_string(),
        content: content.to_string(),
        cert_hash_public: Some(cert_hash_field),
        content_hash_public: Some(content_hash_field),
    };
    
    // Generate the proof using the SNARK trait method
    let mut rng = ark_std::rand::thread_rng();
    let proof = match Groth16::<Bn254>::prove(&proving_key, circuit, &mut rng) {
        Ok(p) => p,
        Err(_) => return String::from("Failed to generate proof"),
    };
    
    // Format proof for Garaga
    format_proof_for_garaga(&proof, &[cert_hash_field, content_hash_field])
}

// Format proof for Garaga - matching the exact format shown in examples
fn format_proof_for_garaga(proof: &Proof<Bn254>, public_inputs: &[Fr]) -> String {
    // Format exactly as in the example with the "1" for z-coordinate
    let pi_a = vec![
        proof.a.x.to_string(),
        proof.a.y.to_string(),
        "1".to_string(),
    ];
    
    // For G2 points, format exactly as in the example
    let pi_b = vec![
        vec![
            proof.b.x.c0.to_string(),
            proof.b.x.c1.to_string(),
        ],
        vec![
            proof.b.y.c0.to_string(),
            proof.b.y.c1.to_string(),
        ],
        vec![
            "1".to_string(),
            "0".to_string(),
        ],
    ];
    
    let pi_c = vec![
        proof.c.x.to_string(),
        proof.c.y.to_string(),
        "1".to_string(),
    ];
    
    // Convert public inputs to strings
    let public_signals: Vec<String> = public_inputs
        .iter()
        .map(|input| input.to_string())
        .collect();
    
    // Create the Garaga-compatible proof
    let garaga_proof = GaragaProof {
        pi_a,
        pi_b,
        pi_c,
        protocol: "groth16".to_string(),
        curve: "bn128".to_string(), // Garaga expects "bn128" for BN254
        public_signals,
    };
    
    // Serialize to JSON
    match serde_json::to_string_pretty(&garaga_proof) {
        Ok(s) => s,
        Err(_) => String::from("Failed to serialize proof to JSON"),
    }
}

// Helper to format verification key in the exact format required by Garaga
fn generate_verification_key_json(vk: &VerifyingKey<Bn254>) -> Result<String, Box<dyn std::error::Error>> {
    use serde_json::json;
    
    // G1 points (alpha_g1)
    let alpha_g1_x = vk.alpha_g1.x.to_string();
    let alpha_g1_y = vk.alpha_g1.y.to_string();
    
    // G2 points (beta_g2, gamma_g2, delta_g2)
    let beta_g2_x0 = vk.beta_g2.x.c0.to_string();
    let beta_g2_x1 = vk.beta_g2.x.c1.to_string();
    let beta_g2_y0 = vk.beta_g2.y.c0.to_string();
    let beta_g2_y1 = vk.beta_g2.y.c1.to_string();
    
    let gamma_g2_x0 = vk.gamma_g2.x.c0.to_string();
    let gamma_g2_x1 = vk.gamma_g2.x.c1.to_string();
    let gamma_g2_y0 = vk.gamma_g2.y.c0.to_string();
    let gamma_g2_y1 = vk.gamma_g2.y.c1.to_string();
    
    let delta_g2_x0 = vk.delta_g2.x.c0.to_string();
    let delta_g2_x1 = vk.delta_g2.x.c1.to_string();
    let delta_g2_y0 = vk.delta_g2.y.c0.to_string();
    let delta_g2_y1 = vk.delta_g2.y.c1.to_string();
    
    // Compute pairing e(alpha, beta)
    let alpha_beta_gt = ark_bn254::Bn254::pairing(&vk.alpha_g1, &vk.beta_g2);
    
    // Format the Fp12 element (vk_alphabeta_12)
    let ab_c0 = alpha_beta_gt.0.c0;
    let ab_c1 = alpha_beta_gt.0.c1;
    
    let vk_alphabeta_12 = json!([
        [
            [
                ab_c0.c0.c0.to_string(),
                ab_c0.c0.c1.to_string()
            ],
            [
                ab_c0.c1.c0.to_string(),
                ab_c0.c1.c1.to_string()
            ],
            [
                ab_c0.c2.c0.to_string(),
                ab_c0.c2.c1.to_string()
            ]
        ],
        [
            [
                ab_c1.c0.c0.to_string(),
                ab_c1.c0.c1.to_string()
            ],
            [
                ab_c1.c1.c0.to_string(),
                ab_c1.c1.c1.to_string()
            ],
            [
                ab_c1.c2.c0.to_string(),
                ab_c1.c2.c1.to_string()
            ]
        ]
    ]);
    
    // IC array (gamma_abc_g1)
    let mut ic = Vec::new();
    for point in &vk.gamma_abc_g1 {
        ic.push(json!([
            point.x.to_string(),
            point.y.to_string(),
            "1"
        ]));
    }
    
    // Create the exact structure with the right fields for Garaga
    let vk_json = json!({
        "protocol": "groth16",
        "curve": "bn128",
        "nPublic": vk.gamma_abc_g1.len() - 1,
        "vk_alpha_1": [alpha_g1_x, alpha_g1_y, "1"],
        "vk_beta_2": [
            [beta_g2_x0, beta_g2_x1],
            [beta_g2_y0, beta_g2_y1],
            ["1", "0"]
        ],
        "vk_gamma_2": [
            [gamma_g2_x0, gamma_g2_x1],
            [gamma_g2_y0, gamma_g2_y1],
            ["1", "0"]
        ],
        "vk_delta_2": [
            [delta_g2_x0, delta_g2_x1],
            [delta_g2_y0, delta_g2_y1],
            ["1", "0"]
        ],
        "vk_ic": ic,
        "vk_alphabeta_12": vk_alphabeta_12,
        "IC": ic,
        
        // Add duplicate fields WITHOUT the vk_ prefix for Garaga to find
        "alpha": [alpha_g1_x, alpha_g1_y, "1"],
        "beta": [
            [beta_g2_x0, beta_g2_x1],
            [beta_g2_y0, beta_g2_y1],
            ["1", "0"]
        ],
        "gamma": [
            [gamma_g2_x0, gamma_g2_x1],
            [gamma_g2_y0, gamma_g2_y1],
            ["1", "0"]
        ],
        "delta": [
            [delta_g2_x0, delta_g2_x1],
            [delta_g2_y0, delta_g2_y1],
            ["1", "0"]
        ]
    });
    
    Ok(serde_json::to_string_pretty(&vk_json)?)
}

// Now implement the new function to generate keys with real parameters
pub fn generate_keys(
    cert_hash: &[u8],
    headers_json: &str,
    content: &str
) -> String {
    // Calculate field elements exactly as done during proof generation
    let cert_hash_field = bytes_to_field(cert_hash);
    
    let mut content_hasher = Sha256::new();
    content_hasher.update(content.as_bytes());
    let content_hash = content_hasher.finalize();
    let _content_hash_field = bytes_to_field(&content_hash);
    
    // Create circuit with real parameters
    let circuit = WebContentCircuit {
        cert_hash: cert_hash.to_vec(),
        headers: headers_json.to_string(),
        content: content.to_string(),
        cert_hash_public: Some(cert_hash_field),
        content_hash_public: Some(_content_hash_field),
    };
    
    // Generate parameters with fixed seed for reproducibility
    let mut rng = ark_std::rand::rngs::StdRng::seed_from_u64(12345);
    
    // Generate the proving parameters
    let params = match Groth16::<Bn254>::generate_random_parameters_with_reduction(
        circuit.clone(), &mut rng
    ) {
        Ok(p) => p,
        Err(_) => return String::from("Error: Failed to generate parameters")
    };
    
    // Serialize proving key
    let mut pk_bytes = Vec::new();
    if params.serialize_uncompressed(&mut pk_bytes).is_err() {
        return String::from("Error: Failed to serialize proving key");
    }
    
    // Generate verification key JSON
    let vk_json = match generate_verification_key_json(&params.vk) {
        Ok(vk) => vk,
        Err(_) => return String::from("Error: Failed to generate verification key JSON")
    };
    
    // Return both keys in a JSON object
    let result = serde_json::json!({
        "proving_key_base64": base64::encode(&pk_bytes),
        "verification_key_json": vk_json
    });
    
    match serde_json::to_string(&result) {
        Ok(s) => s,
        Err(_) => String::from("Error: Failed to serialize keys to JSON")
    }
}

// Extract public inputs in the exact array format shown in the example
pub fn extract_public_inputs(cert_hash: &[u8], content: &str) -> String {
    // Compute the same public inputs used during proof generation
    let cert_hash_field = bytes_to_field(cert_hash);
    
    // Hash the content
    let mut content_hasher = Sha256::new();
    content_hasher.update(content.as_bytes());
    let content_hash = content_hasher.finalize();
    let content_hash_field = bytes_to_field(&content_hash);
    
    // Format as a simple array with BOTH inputs
    let public_inputs = vec![
        cert_hash_field.to_string(),
        content_hash_field.to_string()
    ];
    
    // Return in the exact format shown in the example
    match serde_json::to_string_pretty(&public_inputs) {
        Ok(s) => s,
        Err(_) => String::from("Error: Failed to serialize public inputs to JSON")
    }
}
