open Bghelper.Cli;
open Bghelper.Types;

let (argc, args) = init();

let maybeNginxConfPath = get_string_opt(["--conf", "-c"], args);
let maybePort1 = get_int_opt(["--port1", "-p"], args);
let maybePort2 = get_int_opt(["--port2", "-q"], args);
let maybeServerName = get_string_opt(["--servername", "-n"], args);
let maybeListen = get_int_opt(["--listen", "-l"], args);
let maybeHelp = get_bool_opt(["--help", "-h"], args);
let maybeHealthCheck = get_string_opt(["--healthcheck", "-e"], args);
let currentPort = get_set_bool(["--current-port", "-i"], args); // -i as inspect
let force = get_set_bool(["--force", "-f"], args);

let showHelp = () => print_endline({helptext|
Usage:
======
bgHelperApp.exe [-h] [-p 3000] [-q 3001] [-n lol.com] [-e false] -c /path/to/nginx/conf

When calling bgHelperApp it's possible to pass some options:
--help (-h)             : this help text
--port1 <port>          : port 1 (default to 3000)
--port2 <port>          : port 2 (default to 3001)
--servername <name>     : server name (default to localhost)
--listen <port>         : listen port (default to 80)
--healthcheck           : path to healthcheck program (default to "true")
--current-port          : returns the current configured port and exit.
--force                 : creates the configuration file if doesn't exist (using port1)

|helptext},
);

let withDefault = (opt, def) => switch(opt) {
    | None => def
    | _ => opt
};

let finish = () => {
    let cmdOptions = {
        port1: withDefault(maybePort1, Some(3000)),
        port2: withDefault(maybePort2, Some(3001)),
        servername: withDefault(maybeServerName, Some("localhost")),
        listen: withDefault(maybeListen, Some(80)),
        conf: withDefault(maybeNginxConfPath, Some("/usr/local/etc/nginx/servers/lol.conf")),
        healthcheck: withDefault(maybeHealthCheck, Some("true")),
        force: Some(force),
    };
    switch currentPort {
    | false => Bghelper.Util.process(cmdOptions)
    | true => Printf.printf("%s", string_of_int(Bghelper.Util.inspect(cmdOptions.conf)))
    };
};

switch maybeHelp {
    | None => finish()
    | Some(_) => showHelp()
};