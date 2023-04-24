fn vander(x: f64, N: Option<i32>, increasing: bool) -> Vec<f64> {
    let mut v = Vec::new();
    for i in 0..n {
        v.push(x.powi(i as i32));
    }
    v
}



fn polyfit(x: &[f64], y: &[f64], n: usize) -> (Vec<f64>, Vec<f64>, Vec<f64>) {
    let order = n + 1;

    if n < 0 {
        panic!("n must be non-negative");
    }
    // if x.dimensions != 1 {
    //     panic!("x must be 1-dimensional");
    // }
    if x.len() == 0 {
        panic!("x must not be empty");
    }
    // if y.dimensions() < 1 || y.dimensions() > 2 {
    //     panic!("y must be 1- or 2-dimensional");
    // }
    if y.len() != x.len() {
        panic!("x and y must have the same length");
    }
    
}



fn main() {
    println!("Hello, world!");
    let x = vec![0.0, 1.0, 2.0, 3.0, 4.0, 5.0];
    let y = vec![0.0, 0.8, 0.9, 0.1, -0.8, -1.0];
    let z = polyfit(&x, &y, 3);
    println!("{:?}", z);
}
