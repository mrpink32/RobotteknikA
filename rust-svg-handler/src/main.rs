use std::ffi::*;

fn main() {
    println!("Hello, world!");
    
    let path = CString::new("/mnt/DATA/Repositories/RobotteknikA/svg-lib/test1.svg")
        .unwrap()
        .into_raw();
    let content = unsafe { CStr::from_ptr(svglib::open_file(path)) }
        .to_str()
        .unwrap();
    println!("{}", content);

}
