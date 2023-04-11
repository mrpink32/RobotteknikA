use std::{fs::File, io::Read};
use core::ffi::*;

#[repr(C)]
pub struct Rect {
    x: f64,
    y: f64,
    width: f64,
    height: f64,
    rx: f64,
    ry: f64,
    style: *const c_char,
}

#[repr(C)]
pub struct Circle {
    pub cx: f64,
    pub cy: f64,
    pub r: f64,
    style: *const c_char,
}

#[repr(C)]
pub struct Ellipse {
    pub cx: f64,
    pub cy: f64,
    pub rx: f64,
    pub ry: f64,
    style: *const c_char,
}

#[repr(C)]
pub struct Line {
    pub x1: f64,
    pub y1: f64,
    pub x2: f64,
    pub y2: f64,
}

#[repr(C)]
pub struct Polyline {
    pub x: f64,
    pub y: f64,
    // pub points: *const Point,
    pub npoints: usize,
}

#[repr(C)]
pub struct Polygon {
    pub x: f64,
    pub y: f64,
    // pub points: *const Point,
    pub npoints: usize,
}

#[repr(C)]
pub struct Path {
    pub x: f64,
    pub y: f64,
    // pub points: *const Point,
    pub npoints: usize,
}

#[repr(C)]
pub struct Text {
    pub x: f64,
    pub y: f64,
    pub text: *const c_char,
}



#[no_mangle]
pub extern "C" fn open_file(path: *const c_char) -> *mut c_char {
    let path = unsafe { CStr::from_ptr(path) }.to_str().unwrap();
    let mut file = File::open(std::path::Path::new(path)).expect("Unable to open file!");
    let mut contents = String::new();
    file.read_to_string(&mut contents).unwrap();
    println!("{}", contents);
    let result = std::ffi::CString::new(contents).expect("Unable to create CString!").into_raw();
    result
}
