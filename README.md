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


The helper was written in reason native, to be fast and independent by your code, you can use it with node, go, php or even a fastcgi server.
The new version (V2) has been reimplemented in Rust because of issue of build reproducibility and the fact that Reason is in dormant (dead?) status.

The helper will fail if you don't have correct permissions, so ensure you can write the configuration first, 
it will also tell you if your configuration doesn't exist and you can pass the `--force` parameter to create an initial one.


The helper will bail out with error in case of failed healthcheck, you can use this in your ci/cd environment 
to know if something fails and provide 0 downtime. 


New in V2
===

V2 expect a simpler configuration, expecting something like 
`import _conf_` inside a nginx `server` directive, this way is easier to set up multiple services in a machine listening on different routes.

V2 has therefore dropped the `--servername`, and the `--listen` option.
V2 added a `--route [-r]` option to define a different route from `/`

about HTTPS: 
--
Given V2 new configuration it's compatible with HTTPS as it is.

Known issues: 
--
V2 version has still 0 unit tests, sorry.

```
Usage:
======
bghelper [--help] [-p 3000] [-q 3001] [-e false] -c /path/to/nginx/conf

When calling bgHelperApp it's possible to pass some options:
--help (-h)             : this help text
--port1 <port>          : port 1 (default to 3000)
--port2 <port>          : port 2 (default to 3001)
--healthcheck           : path to healthcheck program (default to "true")
--current-port          : returns the current configured port and exit.
--force                 : creates the configuration file if doesn't exist (using port1)
--route                 : allow a different route from /

```

## Developing:

```
cargo build
```

## Running Binary:
```
cargo run -- <your options here>
```
## Running Tests:
:-(

License:
`bghelper` is licensed under MIT license.
