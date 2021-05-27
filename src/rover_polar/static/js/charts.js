const url = "http://127.0.0.1:5000/data";

$(".dropdown-item").click(function() {
    let selected = $(this).text();
    let dataName = selected;
    let units;
    let color;
    // Find which data was selected
    if (selected === "Probe Temperature") {
        selected = "probeTemp";
        units = " [Celsius]";
        color = "#df2935";
    } else if (selected ==="BME Temperature") {
        selected = "bmeTemp";
        units = " [Celsius]";
        color = "#df2935"
    } else if (selected === "Humidity") {
        selected = "bmeHumid";
        units = " [%]";
        color = "#3772ff"
    } else if (selected === "Altitude") {
        selected = "bmeAlt";
        units = " [Meter]";
        color = "#61E294"
    } else {
        selected = "bmeAir";
        units = " [hPa]";
        color = "#080708";
    }
    $(this).parents(".dropdown").find('.btn').html(dataName);
    // Find the dataset based on the type of data selected
    $.getJSON(url, function(data){
        // Update the padding for the jumbotron
        // Get the correct context to put the graph in
        let ctx = document.getElementById('myChart').getContext('2d');
        ctx.canvas.width = screen.availWidth*.58;
        ctx.canvas.height = screen.availHeight*.58;
        Chart.defaults.global.defaultFontSize = 16;
        // Create the graph based on the current data

        if (dataName = 'Probe Temperature' || 'BME Temperature') {
            document.querySelector('#increasing').innerHTML = `Temperature ${data.data.increasing}`
        } else {
            document.querySelector('#increasing').innerHTML = 'none'
        }

        let myChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: data.data.timestamps,
            datasets: [{
                label: dataName + units,
                data: data.data[selected],
                backgroundColor: color,
                borderColor: color,
                fill: false
                }]
        },
        options: {
            hover: {
             mode: 'index',
             intersect: true
            },
            responsive: false,
            maintainAspectRatio: true
          }
        });
    });
});
