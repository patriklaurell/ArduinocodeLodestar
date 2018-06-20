$(document).ready(function () {
    console.log("ready");
    var socket = io.connect('http://' + document.domain + ':' + location.port + '/test');
    socket.on('connect', function() {
        console.log("connected");
        console.log(socket.connected)
    });
    socket.on('newdata', function(msg) {
        console.log("newdata");
        console.log(msg.data);
        $("#v1").text(msg.data.v1);
        $("#i1").text(msg.data.i1);
        $("#v2").text(msg.data.v2);
        $("#i2").text(msg.data.i2);
    });
})
