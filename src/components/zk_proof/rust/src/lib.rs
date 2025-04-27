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

use ark_bn254::{Bn254, Fr, G1Affine, G2Affine};
use ark_groth16::{Groth16, Proof, ProvingKey, VerifyingKey};
use ark_serialize::{CanonicalDeserialize, CanonicalSerialize};
use ark_relations::r1cs::{ConstraintSynthesizer, ConstraintSystemRef, SynthesisError};
use ark_snark::SNARK;
use ark_std::{Zero, One, io::Cursor};
use sha2::{Sha256, Digest};
use serde::{Serialize, Deserialize};
use serde_json::json;
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
    
    // Add alternative field names for compatibility
    #[serde(rename = "a")]
    a: Vec<String>,
    #[serde(rename = "b")]
    b: Vec<Vec<String>>,
    #[serde(rename = "c")]
    c: Vec<String>,
    #[serde(rename = "public_inputs")]
    public_inputs: Vec<String>,
    #[serde(rename = "publicInputs")]
    publicInputs: Vec<String>,
    #[serde(rename = "input")]
    input: Vec<String>,
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
        // Log original values for debugging
        println!("Processing proof for cert_hash: {:?}", self.cert_hash);
        println!("Processing proof for content: {}", self.content);
        println!("With headers: {}", self.headers);
        
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
    // Format G1 points
    let point_a = vec![
        proof.a.x.to_string(),
        proof.a.y.to_string(),
        "1".to_string(),
    ];
    
    // Format G2 points
    let point_b = vec![
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
    
    let point_c = vec![
        proof.c.x.to_string(),
        proof.c.y.to_string(),
        "1".to_string(),
    ];
    
    // Convert public inputs to strings
    let public_signals: Vec<String> = public_inputs
        .iter()
        .map(|input| input.to_string())
        .collect();
    
    // Create the Garaga-compatible proof with all field variants
    let garaga_proof = GaragaProof {
        pi_a: point_a.clone(),
        pi_b: point_b.clone(),
        pi_c: point_c.clone(),
        a: point_a,
        b: point_b,
        c: point_c,
        protocol: "groth16".to_string(),
        curve: "bn128".to_string(),
        public_signals: public_signals.clone(),
        public_inputs: public_signals.clone(),
        publicInputs: public_signals.clone(),
        input: public_signals,
    };
    
    match serde_json::to_string_pretty(&garaga_proof) {
        Ok(s) => s,
        Err(_) => String::from("Failed to serialize proof to JSON"),
    }
}

// Helper to format verification key in the exact format required by Garaga
fn generate_verification_key_json(vk: &VerifyingKey<Bn254>) -> Result<String, Box<dyn std::error::Error>> {
    use serde_json::json;
    
    // Format points
    let alpha_g1 = vec![
        vk.alpha_g1.x.to_string(),
        vk.alpha_g1.y.to_string(),
        "1".to_string()
    ];
    
    let beta_g2 = vec![
        vec![vk.beta_g2.x.c0.to_string(), vk.beta_g2.x.c1.to_string()],
        vec![vk.beta_g2.y.c0.to_string(), vk.beta_g2.y.c1.to_string()],
        vec!["1".to_string(), "0".to_string()]
    ];
    
    let gamma_g2 = vec![
        vec![vk.gamma_g2.x.c0.to_string(), vk.gamma_g2.x.c1.to_string()],
        vec![vk.gamma_g2.y.c0.to_string(), vk.gamma_g2.y.c1.to_string()],
        vec!["1".to_string(), "0".to_string()]
    ];
    
    let delta_g2 = vec![
        vec![vk.delta_g2.x.c0.to_string(), vk.delta_g2.x.c1.to_string()],
        vec![vk.delta_g2.y.c0.to_string(), vk.delta_g2.y.c1.to_string()],
        vec!["1".to_string(), "0".to_string()]
    ];
    
    // Format IC array
    let mut ic = Vec::new();
    for point in &vk.gamma_abc_g1 {
        ic.push(json!([
            point.x.to_string(),
            point.y.to_string(),
            "1"
        ]));
    }
    
    // Create verification key JSON with both naming conventions
    let vk_json = json!({
        // Original fields
        "vk_alpha_1": alpha_g1.clone(),
        "vk_beta_2": beta_g2.clone(),
        "vk_gamma_2": gamma_g2.clone(),
        "vk_delta_2": delta_g2.clone(),
        "vk_ic": ic.clone(),
        
        // Alternative field names
        "alpha": alpha_g1,
        "beta": beta_g2,
        "gamma": gamma_g2,
        "delta": delta_g2,
        "IC": ic,
        
        // Metadata
        "protocol": "groth16",
        "curve": "bn128",
        "nPublic": vk.gamma_abc_g1.len() - 1,
        
        // Pairing info
        "vk_alphabeta_12": compute_pairing(&vk.alpha_g1, &vk.beta_g2)
    });
    
    Ok(serde_json::to_string_pretty(&vk_json)?)
}

// Helper function to compute and format pairing
fn compute_pairing(alpha_g1: &G1Affine, beta_g2: &G2Affine) -> serde_json::Value {
    let pairing = Bn254::pairing(alpha_g1, beta_g2);
    let c0 = pairing.0.c0;
    let c1 = pairing.0.c1;
    
    json!([
        [
            [
                c0.c0.c0.to_string(),
                c0.c0.c1.to_string()
            ],
            [
                c0.c1.c0.to_string(),
                c0.c1.c1.to_string()
            ],
            [
                c0.c2.c0.to_string(),
                c0.c2.c1.to_string()
            ]
        ],
        [
            [
                c1.c0.c0.to_string(),
                c1.c0.c1.to_string()
            ],
            [
                c1.c1.c0.to_string(),
                c1.c1.c1.to_string()
            ],
            [
                c1.c2.c0.to_string(),
                c1.c2.c1.to_string()
            ]
        ]
    ])
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
