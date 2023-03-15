use std::io::Write;
use std::{path::PathBuf, process::exit};
use clap::Parser;

use std::fs;
use std::process::Command;

use regex::Regex;
use colored::*;

#[derive(Parser,Default,Debug)]
#[clap(author="Cristian Carlesso <@kentaromiura>", version="v2.0.0", about="A Blue/Green deployment helper")]
struct Arguments {
   #[clap(short='c', long="conf")]
   conf: PathBuf,
   #[clap(short='p', long="port1", default_value_t=3000)]
   port1: u32,
   #[clap(short='q', long="port2", default_value_t=3001)]
   port2: u32,
   #[clap(short='e', long="healtcheck", default_value_t=String::from("true"))]
   healthcheck: String,
   #[clap(short='f', long="force", action = clap::ArgAction::SetTrue)]
   force: bool,
   #[clap(short='r', long="route", default_value_t=String::from("/"))]
   route: String,
   #[clap(short='i', long="current-port", action = clap::ArgAction::SetTrue)]
   current_port: bool,
}


fn success(message: &str) {
    println!("{}", message.green().bold());
}

fn error(message: &str) {
    println!("{}", message.red().bold());
    exit(-1);
}

fn ensure_conf_exists(conf: &PathBuf, force: bool) -> bool  { 
    match conf.exists() {
        true => false,
        false => match force {
            true => true,
            false => {
                error("Configuration file doesn't exists, to create one pass --force.");
                false
            }
        }
  }
}

 fn get_current_port_from_conf(path: &PathBuf) -> u32 {
    
    let maybe_read = fs::read_to_string(path.as_os_str().to_str().unwrap());
    if maybe_read.is_err() {
        let msg = "Error: configuration file not found or not accessible";
        error(msg);
    }
    let text = maybe_read.unwrap();
    
    let regex = Regex::new(r"http://localhost:(\d+)").unwrap();
    let mut maybe_port = None;
   
    for cap in regex.captures_iter(text.as_str()) {
        if maybe_port.is_none() {
            maybe_port = Some(cap[1].to_string());
        }
    }

    if maybe_port.is_none() {
        println!("Error: port not found in nginx file.");
        exit(-1);
    }
    let port: u32 = maybe_port.unwrap().parse().unwrap();
    return port;
}


fn get_new_port (running: u32, port1: u32, port2: u32) -> u32 {
    match running == port1 {
        true => port2,
        false => port1
    }
}

fn execute(exe: String, param: String) -> bool {
    return Command::new(exe)
    .arg(param)
    .status().is_ok();
}

fn execute_2(exe: String, param: String, param2: String) -> bool {
    return Command::new(exe)
    .arg(param)
    .arg(param2)
    .status().is_ok();
}

fn write_file(uri: PathBuf, content:String) {
    let file = std::fs::File::create(uri);
    file.unwrap().write_all(content.as_bytes()).expect("impossible to write the file".red().bold().to_string().as_str());
}

fn update_nginx_configuration_and_reload(new_port:u32, conf:&PathBuf, route: String) {
    let template = format!("
    location {} {{
        proxy_pass http://localhost:{};
    }}", route, new_port);
   

    write_file(conf.clone(), template);

    let reload_succeeded = execute_2("nginx".to_string(), "-s".to_string(), "reload".to_string());
    if reload_succeeded {
        success("Successfully reloaded the configuration.");
    } else {
        error("Error while reloading the configuration.");
    }
}


fn main() {
    
    let args = Arguments::parse();
   
    if args.current_port {
        let port = get_current_port_from_conf(&args.conf);
        println!("current port: {}", port);
        return;
    }

    let create = ensure_conf_exists(&args.conf, args.force);
    let running_port = match create {
        true => 0,
        false => get_current_port_from_conf(&args.conf)
    };

    let new_port = get_new_port(running_port, args.port1, args.port2);
    
    if create {
        let message = format!(
            "{}, using {}{}", 
            "service configuration doesn't exist".yellow().underline(),
            "port1: ".blue().on_bright_blue(),
            format!("{}", args.port1).bold()
        );
        println!("{}", message);
    } else {
        let mut msg = format!("port: {}", running_port);
        
        match running_port == args.port1 {
            true => {
                msg = msg.blue().bold().to_string();
            },
            false => {
                msg = msg.green().bold().to_string(); 
            }
        };
        println!("old service running on {}", msg);

    }
    let mut new_msg = format!("new port: {}", new_port);
    match running_port == args.port1 {
        true => {
            new_msg = new_msg.green().bold().to_string();
        },
        false => {
            new_msg = new_msg.blue().bold().to_string();
        }
    };
    println!("testing {}", new_msg);

    let url = format!("http://localhost:{}", new_port);
    let healthcheck_succeeded = execute(args.healthcheck, url);   
    if healthcheck_succeeded {
        success(
            "Healthcheck succeeded."
        );
        update_nginx_configuration_and_reload(new_port, &args.conf, args.route);
    } else {
        error(
            "Health check failed, bailing out!"
        );
    }
}


