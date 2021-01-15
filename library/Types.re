type options = {
    port1: option(int),
    port2: option(int),
    servername: option(string),
    listen: option(int),
    conf: option(string),
    healthcheck: option(string),
    force: option(bool)
};

exception GenericError(string);
