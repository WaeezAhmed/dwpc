
let userRole = JSON.parse(localStorage.getItem("user") || null)



if(userRole ==="Demo"){
 document.getElementById("inference-page").style.display= "none"
 
}


function getInferenceData(){

// let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
    fetch("/rpc/Config.Get", {
		method: 'GET'
	})
  .then((response)=> {
  
    if(response.status === 200){
        return response.json()
    }
    else if(response.status === 401){
      window.location.href="./login.html"
    }
    else{
        alert("something went wrong");
    }})

		.then(response => {
      console.log(response)

      document.getElementById("aggregation-interval").value =  response.dwpc.interval
      document.getElementById("maximum-distance-delta").value  =  response.dwpc.max_distance_delta
      document.getElementById("minimum-floor-distance").value  =  response.dwpc.min_floor_distance
      document.getElementById("room-capacity").value  =  response.dwpc.capacity
      document.getElementById("periodic-reset-interval").value  =  response.dwpc.periodic_reset
      // document.getElementById("function-mode").value  =  response.dwpc.function_mode
      document.getElementById("measurement-frequency").value  =  response.dwpc.sensor_freq
      document.getElementById("refectivity-xTalk").value  =  response.dwpc.reflectance_percent
      document.getElementById("sample-count-xTalk").value  =  response.dwpc.nb_samples
      document.getElementById("target-distance-xTalk").value  =  response.dwpc.distance_mm
      // document.getElementById("wait-timer").value  =  response.dwpc.wait_timer
      document.getElementById("resolution").value  =  response.dwpc.resolution
      document.getElementById("number-of-target-per-zone").value  =  response.dwpc.numberOfTargetPerZone
      document.getElementById("total-samples-calibration").value  =  response.dwpc.TotalSamplesForRangeCalibration
      document.getElementById("valid-samples-range-calibration").value  =  response.dwpc.validSamplesForRangeCalibration
      // document.getElementById("event-one-way").value  =  response.dwpc.event_one_way
      // document.getElementById("event-other-way").value  =  response.dwpc.event_other_way
      document.getElementById("enter-or-exit-middle").value  =  response.dwpc.enter_or_exit_middle_ok
      document.getElementById("direction-ent-ext").value  =  response.dwpc.direction
      document.getElementById("min-consecutive-count").value  =  response.dwpc.min_consecutive_count
      // document.getElementById("minimum-distance").value  =  response.dwpc.min_distance
      document.getElementById("min-signal").value  =  response.dwpc.min_signal
      document.getElementById("threshold").value  =  response.dwpc.threshold
      document.getElementById("low-conf-det").value  =  response.dwpc.enable_low_confidence_target
      document.getElementById("per-count-thres").value  =  response.dwpc.person_count_threshold 
      
      


      
    })
		.catch(err => console.error(err));
  }


  getInferenceData()









document.getElementById("inference-form").addEventListener("submit", getInferenceForm)


function  getInferenceForm(e){
e.preventDefault()
  let aggregationInterval = document.getElementById("aggregation-interval").value
  let MaxDistanceDelta =  document.getElementById("maximum-distance-delta").value 
  let MinFloorDistance = document.getElementById("minimum-floor-distance").value
  let PeriodicResetInterval= document.getElementById("periodic-reset-interval").value
  // let FunctionMode= document.getElementById("function-mode").value
  let MeasurementFrequency = document.getElementById("measurement-frequency").value
  let RelectivityxTalk =   document.getElementById("refectivity-xTalk").value
  let SampleCountxTalk =    document.getElementById("sample-count-xTalk").value 
  let RoomCapacity= document.getElementById("room-capacity").value
  let TargetDistanceXTalk= document.getElementById("target-distance-xTalk").value 
  let Resoluton= document.getElementById("resolution").value
  
  let TargetZone = document.getElementById("number-of-target-per-zone").value
  let SampleCalibration =   document.getElementById("total-samples-calibration").value 
  let ValidSampleRangeCal =  document.getElementById("valid-samples-range-calibration").value

//   let EventOneWay=  document.getElementById("event-one-way").value
// let EventOtherWay =   document.getElementById("event-other-way").value 
 let EnterOrExitMiddle =   document.getElementById("enter-or-exit-middle").value 
 let MinConsecutiveCount= document.getElementById("min-consecutive-count").value 
//  let MinDistance=   document.getElementById("minimum-distance").value 
let MinSignal =   document.getElementById("min-signal").value 
let Threshold = document.getElementById("threshold").value 
let Direction = document.getElementById("direction-ent-ext").value 
let LowConDet = document.getElementById("low-conf-det").value 
let PerCountThresh = document.getElementById("per-count-thres").value 




// let reserve = document.getElementById("reserve").value



  var inferenceData = JSON.stringify({
    config:{
      dwpc :{
        "interval": parseInt(aggregationInterval),
        "max_distance_delta": parseInt(MaxDistanceDelta),
        "min_floor_distance": parseInt(MinFloorDistance),
        "capacity": parseInt(RoomCapacity),
        "periodic_reset": parseInt(PeriodicResetInterval),
        // "function_mode": parseInt(FunctionMode),
        "sensor_freq":  parseInt(MeasurementFrequency) ,
        "reflectance_percent": parseInt(RelectivityxTalk),
        "nb_samples": parseInt(SampleCountxTalk),  
        "distance_mm":   parseInt(TargetDistanceXTalk),  
        "resolution": parseInt(Resoluton),
        "numberOfTargetPerZone":   parseInt(TargetZone), 
        "TotalSamplesForRangeCalibration":   parseInt(SampleCalibration), 
        "validSamplesForRangeCalibration":   parseInt(ValidSampleRangeCal),
        // "event_one_way": parseInt(EventOneWay),
        // "event_other_way": parseInt(EventOtherWay),
        "enter_or_exit_middle_ok": parseInt(EnterOrExitMiddle),
        "min_consecutive_count": parseInt(MinConsecutiveCount),
        // "min_distance": parseInt(MinDistance),
        "min_signal": parseInt(MinSignal),
        "threshold": parseInt(Threshold),
        "direction": parseInt(Direction),
        "enable_low_confidence_target": parseInt(LowConDet),
        "person_count_threshold": parseInt(PerCountThresh),
        // "reserve": parseInt(reserve)

   
     
    },
    },
    });
    
  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
    fetch("/rpc/Config.Set", {
        method: "POST",
        headers: {
            "Accept": "application/json, text/plain, */*",
            "Content-type": "application/json"
        },
        body: inferenceData
    })
    .then((res)=> {
    if(res.status === 200){
    return res.json()
  
    }
    else if(res.status === 401){
      window.location.href="./login.html"
    }
    else{
        alert("something went wrong")  ;
    }})
    .then((data)=>{

      let new_data = JSON.stringify({ reboot: false })
      fetch("/rpc/Config.Save", {
        method: "POST",
        headers: {
            "Accept": "application/json, text/plain, */*",
            "Content-type": "application/json"
        },
        body: new_data 

    })
    .then((res)=> {
      if(res.status === 200){
      return res.json()
      }
      else if(res.status === 401){
        window.location.href="./login.html"
      }
      else{
          alert("something went wrong")  ;
      }})
      .then((data)=>{
    
        alert("Inference Parameters is set sucessfully");
      })
      .catch(err => console.log(err))


    })
    .catch(err => console.log(err))


    }

