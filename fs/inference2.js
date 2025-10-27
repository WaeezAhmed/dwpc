



    
  function getDataForThreshold(){
  
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

      const temp = [response.distanceThreshold.index0, 
        response.distanceThreshold.index1, 
        response.distanceThreshold.index2,
         response.distanceThreshold.index3, 
         response.distanceThreshold.index4, 
         response.distanceThreshold.index5,
          response.distanceThreshold.index6, 
          response.distanceThreshold.index7, 
          response.distanceThreshold.index8, 
          response.distanceThreshold.index9, 
          response.distanceThreshold.index10, 
          response.distanceThreshold.index11,
           response.distanceThreshold.index12, 
           response.distanceThreshold.index13,
            response.distanceThreshold.index14, 
            response.distanceThreshold.index15
         ]

      const container1 = document.querySelector(".grid-container2");
       container1.innerHTML= ""
      container1.classList.remove("display_none");
         for (let i = 0; i < 16; i++) {
           const item = document.createElement("div");
           item.classList.add("grid-item");
         
        //  let color = Math.floor((response[i]/ 10)+ 60)
              
         let color = Math.floor((temp[i] / 10)+ 60)
           
           item.style.backgroundColor= `hsl(${color}, 50%, 50%)`;
     

           if(i ==0 || i== 3 ||  i==12 || i==15){
            //  item.innerHTML = one_d_array[i] + ` (${i})`
            item.innerHTML=  temp[i] + ` (${i})`
           }
           else{
            item.innerHTML=  temp[i] 
           }

                    
           container1.appendChild(item);
         }

      
    

    })
    .catch(err => console.error(err));
  }

  getDataForThreshold()


  function getZoneImportance_ZoneDirection_DistanceThreshold_Data(){
  
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
  
        document.getElementById("select0").value =  response.pathzone.index0
        document.getElementById("select1").value =  response.pathzone.index1
        document.getElementById("select2").value  =  response.pathzone.index2
        document.getElementById("select3").value  =  response.pathzone.index3
        document.getElementById("select4").value  =  response.pathzone.index4
        document.getElementById("select5").value  =  response.pathzone.index5
        document.getElementById("select6").value =  response.pathzone.index6
        document.getElementById("select7").value = response.pathzone.index7
        document.getElementById("select8").value  =  response.pathzone.index8
        document.getElementById("select9").value  = response.pathzone.index9
        document.getElementById("select10").value  =  response.pathzone.index10
        document.getElementById("select11").value  = response.pathzone.index11
        document.getElementById("select12").value =  response.pathzone.index12
        document.getElementById("select13").value = response.pathzone.index13
        document.getElementById("select14").value  =  response.pathzone.index14
        document.getElementById("select15").value  =  response.pathzone.index15


        document.getElementById("zoneDir0").value =  response.InnerOuterZone.index0
        document.getElementById("zoneDir1").value =  response.InnerOuterZone.index1
        document.getElementById("zoneDir2").value  =  response.InnerOuterZone.index2
        document.getElementById("zoneDir3").value  =  response.InnerOuterZone.index3
        document.getElementById("zoneDir4").value  =  response.InnerOuterZone.index4
        document.getElementById("zoneDir5").value  =  response.InnerOuterZone.index5
        document.getElementById("zoneDir6").value =  response.InnerOuterZone.index6
        document.getElementById("zoneDir7").value =  response.InnerOuterZone.index7
        document.getElementById("zoneDir8").value  =  response.InnerOuterZone.index8
        document.getElementById("zoneDir9").value  =  response.InnerOuterZone.index9
        document.getElementById("zoneDir10").value  =  response.InnerOuterZone.index10
        document.getElementById("zoneDir11").value  =  response.InnerOuterZone.index11
        document.getElementById("zoneDir12").value =  response.InnerOuterZone.index12
        document.getElementById("zoneDir13").value =  response.InnerOuterZone.index13
        document.getElementById("zoneDir14").value  =  response.InnerOuterZone.index14
        document.getElementById("zoneDir15").value  =  response.InnerOuterZone.index15

    
        // console.log(response.distanceThreshold, response.distanceThreshold.index2, response.distanceThreshold.index3)

        document.getElementById("index0").value =  response.distanceThreshold.index0
        document.getElementById("index1").value =  response.distanceThreshold.index1
        document.getElementById("index2").value =  response.distanceThreshold.index2
        document.getElementById("index3").value  =  response.distanceThreshold.index3
        document.getElementById("index4").value  =  response.distanceThreshold.index4
        document.getElementById("index5").value  =  response.distanceThreshold.index5
        document.getElementById("index6").value =  response.distanceThreshold.index6
        document.getElementById("index7").value =  response.distanceThreshold.index7
        document.getElementById("index8").value  =  response.distanceThreshold.index8
        document.getElementById("index9").value  =  response.distanceThreshold.index9
        document.getElementById("index10").value  =  response.distanceThreshold.index10
        document.getElementById("index11").value  =  response.distanceThreshold.index11
        document.getElementById("index12").value =  response.distanceThreshold.index12
        document.getElementById("index13").value =  response.distanceThreshold.index13
        document.getElementById("index14").value  =  response.distanceThreshold.index14
        document.getElementById("index15").value  =  response.distanceThreshold.index15
        

        document.getElementById("minDis0").value =  response.mindistance.index0
        document.getElementById("minDis1").value =  response.mindistance.index1
        document.getElementById("minDis2").value =  response.mindistance.index2
        document.getElementById("minDis3").value  =  response.mindistance.index3
        document.getElementById("minDis4").value  =  response.mindistance.index4
        document.getElementById("minDis5").value  =  response.mindistance.index5
        document.getElementById("minDis6").value =  response.mindistance.index6
        document.getElementById("minDis7").value =  response.mindistance.index7
        document.getElementById("minDis8").value  =  response.mindistance.index8
        document.getElementById("minDis9").value  =  response.mindistance.index9
        document.getElementById("minDis10").value  =  response.mindistance.index10
        document.getElementById("minDis11").value  =  response.mindistance.index11
        document.getElementById("minDis12").value =  response.mindistance.index12
        document.getElementById("minDis13").value =  response.mindistance.index13
        document.getElementById("minDis14").value  =  response.mindistance.index14
        document.getElementById("minDis15").value  =  response.mindistance.index15

      })
      .catch(err => console.error(err));
    }
  
  
    getZoneImportance_ZoneDirection_DistanceThreshold_Data()


    




    //post req

    document.getElementById("zoneImportance").addEventListener("submit", zoneImportance)


    function  zoneImportance(e){
        e.preventDefault()
    


    let pathzoneIndex0 = document.getElementById("select0").value
    let pathzoneIndex1 = document.getElementById("select1").value
    let pathzoneIndex2 = document.getElementById("select2").value
    let pathzoneIndex3 = document.getElementById("select3").value
    let pathzoneIndex4 = document.getElementById("select4").value
    let pathzoneIndex5 = document.getElementById("select5").value
    let pathzoneIndex6 = document.getElementById("select6").value
    let pathzoneIndex7 = document.getElementById("select7").value
    let pathzoneIndex8 = document.getElementById("select8").value
    let pathzoneIndex9 = document.getElementById("select9").value
    let pathzoneIndex10 = document.getElementById("select10").value 
    let pathzoneIndex11 = document.getElementById("select11").value
    let pathzoneIndex12 = document.getElementById("select12").value 
    let pathzoneIndex13 = document.getElementById("select13").value
    let pathzoneIndex14 = document.getElementById("select14").value 
    let pathzoneIndex15 = document.getElementById("select15").value

    
    
      var zoneImportanceData = JSON.stringify({
        config:{
            pathzone :{
            "index0": parseInt(pathzoneIndex0),
            "index1": parseInt(pathzoneIndex1),
            "index2": parseInt(pathzoneIndex2),
            "index3": parseInt(pathzoneIndex3),
            "index4": parseInt(pathzoneIndex4),
            "index5": parseInt(pathzoneIndex5),
            "index6": parseInt(pathzoneIndex6),
            "index7": parseInt(pathzoneIndex7),
            "index8": parseInt(pathzoneIndex8),
            "index9": parseInt(pathzoneIndex9),
            "index10": parseInt(pathzoneIndex10),
            "index11": parseInt(pathzoneIndex11),
            "index12": parseInt(pathzoneIndex12),
            "index13": parseInt(pathzoneIndex13),
            "index14": parseInt(pathzoneIndex14),
            "index15": parseInt(pathzoneIndex15)


        
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
            body: zoneImportanceData
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
        
            alert("Zone-Importance is set sucessfully");
          })
          .catch(err => console.log(err))
    
    
        })
        .catch(err => console.log(err))
    
    
        }
    
    


           //post request
        document.getElementById("zoneDirection").addEventListener("submit", zoneDirection)
    
    
        function  zoneDirection(e){
            e.preventDefault()
        
  
        let InnerOuterZone0 = document.getElementById("zoneDir0").value
        let InnerOuterZone1 = document.getElementById("zoneDir1").value
        let InnerOuterZone2 = document.getElementById("zoneDir2").value
        let InnerOuterZone3 = document.getElementById("zoneDir3").value
        let InnerOuterZone4 = document.getElementById("zoneDir4").value
        let InnerOuterZone5 = document.getElementById("zoneDir5").value
        let InnerOuterZone6 = document.getElementById("zoneDir6").value
        let InnerOuterZone7 = document.getElementById("zoneDir7").value
        let InnerOuterZone8 = document.getElementById("zoneDir8").value
        let InnerOuterZone9 = document.getElementById("zoneDir9").value
        let InnerOuterZone10 = document.getElementById("zoneDir10").value 
        let InnerOuterZone11 = document.getElementById("zoneDir11").value
        let InnerOuterZone12 = document.getElementById("zoneDir12").value 
        let InnerOuterZone13 = document.getElementById("zoneDir13").value
        let InnerOuterZone14 = document.getElementById("zoneDir14").value 
        let InnerOuterZone15 = document.getElementById("zoneDir15").value
        
        
          var zoneDirectionData = JSON.stringify({
            config:{
                InnerOuterZone:{
                    "index0": parseInt(InnerOuterZone0),
                    "index1": parseInt(InnerOuterZone1),
                    "index2": parseInt(InnerOuterZone2),
                    "index3": parseInt(InnerOuterZone3),
                    "index4": parseInt(InnerOuterZone4),
                    "index5": parseInt(InnerOuterZone5),
                    "index6": parseInt(InnerOuterZone6),
                    "index7": parseInt(InnerOuterZone7),
                    "index8": parseInt(InnerOuterZone8),
                    "index9": parseInt(InnerOuterZone9),
                    "index10": parseInt(InnerOuterZone10),
                    "index11": parseInt(InnerOuterZone11),
                    "index12": parseInt(InnerOuterZone12),
                    "index13": parseInt(InnerOuterZone13),
                    "index14": parseInt(InnerOuterZone14),
                    "index15": parseInt(InnerOuterZone15)
                
                },
            },
            });
            
    
            fetch("/rpc/Config.Set", {
                method: "POST",
                headers: {
                    "Accept": "application/json, text/plain, */*",
                    "Content-type": "application/json"
                },
                body: zoneDirectionData 
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
                
                alert("Zone-Direction is set sucessfully");
              })
              .catch(err => console.log(err))
        
        
            })
            .catch(err => console.log(err))
        
        
            }
        
    //post request
            document.getElementById("distancethreshold-form").addEventListener("submit", distancethresholdForm)
    
    
            function  distancethresholdForm(e){
                e.preventDefault()
            
      
            let distanceThreshold0 = document.getElementById("index0").value
            let distanceThreshold1 = document.getElementById("index1").value
            let distanceThreshold2 = document.getElementById("index2").value
            let distanceThreshold3 = document.getElementById("index3").value
            let distanceThreshold4 = document.getElementById("index4").value
            let distanceThreshold5 = document.getElementById("index5").value
            let distanceThreshold6 = document.getElementById("index6").value
            let distanceThreshold7 = document.getElementById("index7").value
            let distanceThreshold8 = document.getElementById("index8").value
            let distanceThreshold9 = document.getElementById("index9").value
            let distanceThreshold10 = document.getElementById("index10").value 
            let distanceThreshold11 = document.getElementById("index11").value
            let distanceThreshold12 = document.getElementById("index12").value 
            let distanceThreshold13 = document.getElementById("index13").value
            let distanceThreshold14 = document.getElementById("index14").value 
            let distanceThreshold15 = document.getElementById("index15").value
            
            
              var distanceThresholdData = JSON.stringify({
                config:{
                  distanceThreshold:{
                        "index0": parseInt(distanceThreshold0),
                        "index1": parseInt(distanceThreshold1),
                        "index2": parseInt(distanceThreshold2),
                        "index3": parseInt(distanceThreshold3),
                        "index4": parseInt(distanceThreshold4),
                        "index5": parseInt(distanceThreshold5),
                        "index6": parseInt(distanceThreshold6),
                        "index7": parseInt(distanceThreshold7),
                        "index8": parseInt(distanceThreshold8),
                        "index9": parseInt(distanceThreshold9),
                        "index10": parseInt(distanceThreshold10),
                        "index11": parseInt(distanceThreshold11),
                        "index12": parseInt(distanceThreshold12),
                        "index13": parseInt(distanceThreshold13),
                        "index14": parseInt(distanceThreshold14),
                        "index15": parseInt(distanceThreshold15)
                    
                    },
                },
                });
                
        
                fetch("/rpc/Config.Set", {
                    method: "POST",
                    headers: {
                        "Accept": "application/json, text/plain, */*",
                        "Content-type": "application/json"
                    },
                    body: distanceThresholdData
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
                
                    alert("distanceThreshold Data is set sucessfully");
                  })
                  .catch(err => console.log(err))
            
            
                })
                .catch(err => console.log(err))
            
            
                }
            

                 //post request
            document.getElementById("minDistance-form").addEventListener("submit", minimumDistanceForm)
    
            function  minimumDistanceForm(e){
                e.preventDefault()
            
      
            let minDistancefield0 = document.getElementById("minDis0").value
            let minDistancefield1 = document.getElementById("minDis1").value
            let minDistancefield2 = document.getElementById("minDis2").value
            let minDistancefield3 = document.getElementById("minDis3").value
            let minDistancefield4 = document.getElementById("minDis4").value
            let minDistancefield5 = document.getElementById("minDis5").value
            let minDistancefield6 = document.getElementById("minDis6").value
            let minDistancefield7 = document.getElementById("minDis7").value
            let minDistancefield8 = document.getElementById("minDis8").value
            let minDistancefield9 = document.getElementById("minDis9").value
            let minDistancefield10 = document.getElementById("minDis10").value 
            let minDistancefield11 = document.getElementById("minDis11").value
            let minDistancefield12 = document.getElementById("minDis12").value 
            let minDistancefield13 = document.getElementById("minDis13").value
            let minDistancefield14 = document.getElementById("minDis14").value 
            let minDistancefield15 = document.getElementById("minDis15").value
            
            
              var minDistanceData = JSON.stringify({
                config:{
                  mindistance:{
                        "index0": parseInt(minDistancefield0),
                        "index1": parseInt(minDistancefield1),
                        "index2": parseInt(minDistancefield2),
                        "index3": parseInt(minDistancefield3),
                        "index4": parseInt(minDistancefield4),
                        "index5": parseInt(minDistancefield5),
                        "index6": parseInt(minDistancefield6),
                        "index7": parseInt(minDistancefield7),
                        "index8": parseInt(minDistancefield8),
                        "index9": parseInt(minDistancefield9),
                        "index10": parseInt(minDistancefield10),
                        "index11": parseInt(minDistancefield11),
                        "index12": parseInt(minDistancefield12),
                        "index13": parseInt(minDistancefield13),
                        "index14": parseInt(minDistancefield14),
                        "index15": parseInt(minDistancefield15)
                    
                    },
                },
                });
                
        
                fetch("/rpc/Config.Set", {
                    method: "POST",
                    headers: {
                        "Accept": "application/json, text/plain, */*",
                        "Content-type": "application/json"
                    },
                    body: minDistanceData
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
                    console.log(data) 
                    alert("distanceThreshold Data is set sucessfully");
                  })
                  .catch(err => console.log(err))
            
            
                })
                .catch(err => console.log(err))
            
            
                }
            


   