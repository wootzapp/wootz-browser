
#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
       
    }
}


    fn main() {
        let x = 10;
        if x == 0 {
            println!("zero!");
        } else if x < 100 {
            println!("biggish");
        } else {
            println!("huge");
        }
    }