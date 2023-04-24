use std::{env, net, path::Path, io::{Read, BufReader, BufWriter, Write}, fs::File};

// fn vander(x: &f64, N: Option<i32>, increasing: bool) -> Vec<f64> {
//     let mut v = Vec::new();
//     for i in 0..n {
//         v.push(x.powi(i as i32));
//     }
//     v
// }
// 
// 
// 
// fn polyfit(x: &[f64], y: &[f64], n: usize, w: Option<Vec<f64>>) -> (Vec<f64>, Vec<f64>, Vec<f64>) {
//     let order = n + 1;
// 
//     if n < 0 {
//         panic!("n must be non-negative");
//     }
//     // if x.dimensions != 1 {
//     //     panic!("x must be 1-dimensional");
//     // }
//     if x.len() == 0 {
//         panic!("x must not be empty");
//     }
//     // if y.dimensions() < 1 || y.dimensions() > 2 {
//     //     panic!("y must be 1- or 2-dimensional");
//     // }
//     if y.len() != x.len() {
//         panic!("x and y must have the same length");
//     }
// 
// 
//     let lhs = vander(x, order, false);
//     let rhs = y;
// 
//     if w.is_some() {
// 
//     }
//     
// }
// let TYPES: Vec<String> = vec!["<rect", "<circle", "<ellipse", "<line", "<polyline", "<polygon", "<squircle", "<polynomial"];

struct SvgHandler {
    width: f64,
    height: f64,
    scale: f64,
    data: Vec<String>,
}

impl SvgHandler {
    fn new(path: &str) -> Self {
        let path = Path::new(path);
        let mut file: File = File::open(path).unwrap();
        let mut buffer = String::new();
        drop(file.read_to_string(&mut buffer));   
        let data: Vec<String> = buffer.split_whitespace().map(|s| s.to_string()).collect();
        let mut width: f64 = 0.0;
        let mut height: f64 = 0.0;
        for token in data.iter() {
            if token.contains("width") {
                let token = token.strip_prefix("width=\"").unwrap();
                let token = token.strip_suffix("\"").unwrap();
                width = token.parse::<f64>().expect("Failed to parse width");
            }
            if token.contains("height") {
                let token = token.strip_prefix("height=\"").unwrap();
                let token = token.strip_suffix("\"").unwrap();
                height = token.parse::<f64>().expect("Failed to parse height");
            }
        }
        let scale = 5.0/(((width / 2.0).powi(2) + (height / 2.0).powi(2)).sqrt());
        SvgHandler {
            width,
            height,
            scale,
            data,
        }
    }

    fn interpret(&mut self) {
        let mut pos_lists: Vec<String> = Vec::new();
        let mut positions: Vec<i32> = Vec::new();
        let mut g = 0;

        for i in 0..self.data.len() {
            if self.data[i].contains("<g") {
                g = 1;
            }
            if self.data[i].contains("</g") {
                g = 0;
                break;
            }
            // if types.contains(self.data[i]) && g == 1 {

            // }
                
            
        }



    }
}



fn main() {
    let mut net_stream = match net::TcpStream::connect("192.168.4.1:1337") {
        Ok(client) => client,
        Err(err) => panic!("Failed to connect"),
    };
    let mut data = String::from("points:?");
    let mut packet = data.as_bytes();
    net_stream.write_all(packet);
    



    let args: Vec<String> = env::args().collect();
    println!("{:?}", args);
    // let mut svg_handler = SvgHandler::new(&args[1]);
    // svg_handler.interpret();





    // println!("Hello, world!");
    // let x = vec![0.0, 1.0, 2.0, 3.0, 4.0, 5.0];
    // let y = vec![0.0, 0.8, 0.9, 0.1, -0.8, -1.0];
    // let z = polyfit(&x, &y, 3);
    // println!("{:?}", z);
}
