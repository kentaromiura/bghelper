open Types;

let error = message => {
    print_endline(Pastel.(
        <Pastel bold=true color=Red>
            message
        </Pastel>
    ));
};

let success = message => {
print_endline(Pastel.(
        <Pastel bold=true color=Green>
            message
        </Pastel>
    ));
};


let unwrap = opt => switch(opt) {
    | Some(x) => x
    | None => raise(Invalid_argument("unwrap")) 
};

let readAllFile = filename => {
    let inChannel = open_in(filename);
    let s = really_input_string(inChannel, in_channel_length(inChannel));
    close_in(inChannel);
    s
};

let writeFile = (filePath: string, content: string) => {
    try ({
        let outFile = open_out(filePath);
        output_string(outFile, content);
        close_out(outFile);
    }) {
        | _ => {
            error("Can't write the configuration file, ensure proper permission are given.");
            raise(GenericError("Can't write file"));
        }

    }
};

let getCurrentPortFromConf = file => {
    let regex = Str.regexp(".*http://localhost:\\([0-9]+\\);.*");
    let _ = Str.search_forward(regex, file, 0);
    int_of_string(Str.matched_group(1, file));
};

let inspect = conf => {
    let conf = unwrap(conf);
    let current = readAllFile(conf);
    getCurrentPortFromConf(current);
};

let getNewPort = (running, port1, port2) => {
    switch (running == port1) {
    | true => port2
    | false => port1
    }
};

let updateNginxConfigurationAndReload = (listen, servername, newport, conf) => {
    let template = Printf.sprintf({tpl|
        server {
            listen %d;
            server_name %s;
            location / {
                proxy_pass http://localhost:%d;
            }
        }
    |tpl}, listen, servername, newport);
    writeFile(conf, template);
    let reloadSucceeded = Sys.command("nginx -s reload") == 0;
    if (reloadSucceeded) {
        success("Successfully reloaded the configuration.");
    } else {
        error("Error while reloading the configuration.");
        raise(GenericError("New service is not up"));
    }
}

let ensureConfExists = (conf, force) => {
    switch (Sys.file_exists(conf)) {
        | true => false
        | false => switch(force) {
            | true => true
            | false => {
                error("Configuration file doesn't exists, to create one pass --force.");
                raise(GenericError("No config"));
            }
        }
    }
};

let process = options => {
    let port1 = unwrap(options.port1);
    let port2 = unwrap(options.port2);
    let servername = unwrap(options.servername);
    let listen = unwrap(options.listen);
    let conf = unwrap(options.conf);
    let healthcheck = unwrap(options.healthcheck);
    let force = unwrap(options.force);

    let create = ensureConfExists(conf, force);
    let runningPort = switch(create) {
        | true => 0
        | false => getCurrentPortFromConf(readAllFile(conf));
    };

    let newPort = getNewPort(runningPort, port1, port2);

    if (create) {
        let message = Pastel.(
        <Pastel underline=true color=Yellow>
            "service doesn't exist"
            <Pastel>
                ", using "
                <Pastel color=Blue>
                    "port1: "
                    <Pastel bold=true>
                        (string_of_int(port1))
                    </Pastel>
                </Pastel>
            </Pastel>
        </Pastel>
    )
        print_endline(message);
    } else {
        let color = switch(runningPort == port1) {
            | true =>  (message) => Pastel.(<Pastel color=Blue bold=true>message</Pastel>)
            | false => (message) => Pastel.(<Pastel color=Green bold=true>message</Pastel>)
        };
        let colorMessage = color(
            "port: " ++ string_of_int(runningPort) ++ "."
        );
        print_endline(Pastel.(
            <Pastel>
                "old service running on "
                colorMessage      
            </Pastel>
            )
        );
    }
    let opposite = switch(runningPort != port1) {
        | true =>  (message) => Pastel.(<Pastel color=Blue bold=true>message</Pastel>)
        | false => (message) => Pastel.(<Pastel color=Green bold=true>message</Pastel>)
    };
        
    let oppositeMessage = opposite("new port: " ++ string_of_int(newPort) ++ ".");
    print_endline(Pastel.(
        <Pastel>
            "testing "
            oppositeMessage
        </Pastel>
    ));

    let healthCheckSucceeded = Sys.command(healthcheck ++ " http://localhost:" ++ string_of_int(newPort)) == 0;
    if (healthCheckSucceeded) {
        success(
            "Healthcheck succeeded."
        );
        updateNginxConfigurationAndReload(listen, servername, newPort, conf);
    } else {
        error(
            "Health check failed, bailing out!"
        );
        raise(GenericError("Service is not up"));
    }
};