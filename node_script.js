var request = require('request');


let jsons = [
    {
      "id": 81,
      "configId": 46,
      "modelID": "flame_detection",
      "cameraMake": "AXIS",
      "color": "HSV:(0,255);(0,255);(0,255);",
      "enable": true,
      "roiCoords": "(575,45);(635,95);",
      "region": "Plant1",
      "threshold": "5",
      "camID": "AXIS0019",
      "assetOwner": "Ashith",
      "status": 0,
      "ipAddress": "/home/allahbaksh/workspaces/junk/vsd_videos/flame.mkv",
      "location": "Goa"
    }
];

var offset = 0;
jsons.map( json_body => {
    
    setTimeout(()=>{
        request.post(
            'http://localhost:9080/start/',
            { 'json': json_body },
            function (error, response, body) {
                if (!error && response.statusCode == 200) {
                    console.log(body)
                }
            }
        );
        //console.log('Thread started')
    },1000+offset)
    offset+=1000
})
