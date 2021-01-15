# bghelper

`bgHelper` is a simple helper to use with nginx to have a working [blue/green deployment](https://en.wikipedia.org/wiki/Blue-green_deployment).
Just spawn your own server on the opposite port and run the tool, it will refresh the nginx configuration if the healthcheck succeed.
If it succeed you can safely tear down the previous instance.


The healthcheck command is configurable, it will be called by passing the localhost:port of the new service.

By default it uses unix "true" which bypass it, I'll suggest to use at least curl `--healthcheck curl` to ensure the server is up.

`curl` can still be returning false positive if the server returns a custom 500 page for example, and curl might not be available on your machine.


If you don't know which port to use for your new instance, fear not! 

You can the `-i` flag to inspect what is the current port in the nginx configuration.


Since you can control the ports the helper uses (with `-p` and `-q`), you can automate spawn your server on the other.


The helper is written in reason native, to be fast and independent by your code, you can use it with node, go, php or even a fastcgi server.


The helper will fail if you don't have correct permissions, so ensure you can write the configuration first, 
it will also tell you if your configuration doesn't exist and you can pass the `--force` parameter to create an initial one.


The helper will bail out with error in case of failed healthcheck, you can use this in your ci/cd environment 
to know if something fails and provide 0 downtime. 


** known issues **

Currently the helper can only setup http server, an option for ssl might be added in the future.
First version has 0 unit tests, sorry.

```
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


```

## Developing:

```
npm install -g esy
git clone <this-repo>
esy install
esy build
```

## Running Binary:

After building the project, you can run the main binary that is produced.

```
esy x BghelperApp.exe 
```

## Running Tests:

```
# Runs the "test" command in `package.json`.
esy test
```

License:
`bghelper` is licensed under MIT license.
It uses [`kentaromiura/minicli`](https://github.com/kentaromiura/minicli) which is licensed as LGPL as per original [https://github.com/UnixJunkie/minicli](`unixjunkie/minicli`).
