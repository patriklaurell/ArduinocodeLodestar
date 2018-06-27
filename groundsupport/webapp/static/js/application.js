$(document).ready(function () {
    console.log("ready");
    var socket = io.connect('http://' + document.domain + ':' + location.port + '/test');
    socket.on('connect', function() {
        console.log("connected");
        console.log(socket.connected)
    });
    socket.on('new_data', function(msg) {
        console.log("new_data");
        console.log(msg.data);
        let v = msg.data.v;
        let i = msg.data.i;
        let cell_nr = msg.data.cell_nr;
        $("#c" + (cell_nr) +"v").text(v);
        $("#c" + (cell_nr) +"i").text(i);
        $("#c" + (cell_nr)).css('color', 'white');
        $("#c" + (cell_nr)).css('background-color', 'green');
    });
    socket.on('new_metadata', function(msg) {
        console.log("new_metadata");
        console.log(msg.data);
        let frame_nr = msg.data.frame;
        let time = msg.data.time;
        let temp = msg.data.temp;
        let rad = msg.data.rad;
        $("#time").text(time);
        $("#temp").text(temp);
        $("#rad").text(rad);
        $("#frame_nr").text(frame_nr);
        $("#time_since_last").text(0);
        $("h3").css('color', '');
        $("h3").css('background-color', '');
    });

    $("#next_button").on("click", function() {
        console.log("clicked next");
        console.log("emitting next_frame event");
        socket.emit('next_frame');
    })

    $("#prev_button").on("click", function() {
        console.log("clicked previous");
        console.log("emitting prev_frame event");
        socket.emit('prev_frame');
    })

    $("#goto_button").on("click", function() {
        console.log("clicked goto");
        console.log("emitting goto_frame event");
        let f = parseInt($("#goto_input").val());
        console.log("goto frame: " + f)
        socket.emit('goto_frame', f);
    })

    setInterval(function() {
        let t = $("#time_since_last").text();
        let tnew = parseInt(t) + 1;
        $("#time_since_last").text(tnew);
    }, 1000);
})
