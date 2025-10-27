

let userRole = JSON.parse(localStorage.getItem("user") || null)


if (userRole === "Demo") {
  document.getElementById("network-page").style.display = "none"

}

let newValue
function getAdminDataNetwork() {

  fetch("/rpc/Config.Get", {
    method: 'GET'

  })
    // .then(response => response.json())
    .then((response) => {

      if (response.status === 200) {
        return response.json()
      }
      else if (response.status === 401) {
        window.location.href = "./login.html"
      }
      else {
        alert("something went wrong");
      }
    })
    .then(response => {


      newValue = response.wifi.sta.enable == true

    })
    .catch(err => console.log(err));
}


getAdminDataNetwork()



function getNetworkData() {


  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
  fetch("/rpc/Config.Get", {
    method: 'GET'
  })
    // .then(response => response.json())
    .then((response) => {

      if (response.status === 200) {
        return response.json()
      }
      else if (response.status === 401) {
        window.location.href = "./login.html"
      }
      else {
        alert("something went wrong");
      }
    })
    .then(response => {


      document.getElementById("network-wifi-ssid").value = response.wifi.sta.ssid
      document.getElementById("network-wifi-password").value = response.wifi.sta.pass

      if (newValue) {
        console.log("newValue**************",newValue);
        document.getElementById("network-static-ip").value = response.wifi.sta.ip
        document.getElementById("network-gateway-ip").value = response.wifi.sta.gw
        document.getElementById("network-netmask").value = response.wifi.sta.netmask
      }
      else {
        document.getElementById("network-static-ip").value = response.eth.ip
        document.getElementById("network-gateway-ip").value = response.eth.gw
        document.getElementById("network-netmask").value = response.eth.netmask
      }


      document.getElementById("ntp-data").value = response.sntp.server

    })
    .catch(err => console.error(err));
}


getNetworkData()







document.getElementById("network-wifi-form").addEventListener("submit", getWifiInferenceForm)


function getWifiInferenceForm(e) {
  e.preventDefault()
  let networkSsid = document.getElementById("network-wifi-ssid").value
  let networkPassword = document.getElementById("network-wifi-password").value


  var networkData = JSON.stringify({
    config: {
      wifi: {
        "sta.ssid": networkSsid,
        "sta.pass": networkPassword
      }
    }


  });

  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
  fetch("/rpc/Config.Set", {
    method: "POST",
    headers: {
      "Accept": "application/json, text/plain, */*",
      "Content-type": "application/json",
      // Authorization: `Bearer ${BearerCheck}`,
    },
    body: networkData
  })
    .then((res) => {
      if (res.status === 200) {
        return res.json()
      } else if (res.status === 401) {
        window.location.href = "./login.html"
      }
      else {
        alert("something went wrong");
      }
    })
    .then((data) => {

      let new_data = JSON.stringify({ reboot: false })
      fetch("/rpc/Config.Save", {
        method: "POST",
        headers: {
          "Accept": "application/json, text/plain, */*",
          "Content-type": "application/json"
        },
        body: new_data

      })
        .then((res) => {
          if (res.status === 200) {
            return res.json()
          }
          else if (res.status === 401) {
            window.location.href = "./login.html"
          }
          else {
            alert("something went wrong");
          }
        })
        .then((data) => {

          alert("Wifi credentials parameter is set sucessfully");
        })
        .catch(err => console.log(err))
    })
    .catch(err => console.log(err))


}





document.getElementById("network-staticip-form").addEventListener("submit", getStaticIpWifiInferenceForm)


function getStaticIpWifiInferenceForm(e) {
  e.preventDefault()
  let networkStaticIp = document.getElementById("network-static-ip").value
  let networkGatewayIp = document.getElementById("network-gateway-ip").value
  let networkNetmask = document.getElementById("network-netmask").value


  let inferenceStaticData
  if (newValue) {
    inferenceStaticData = JSON.stringify({
      config: {
        wifi: {
          "sta.ip": networkStaticIp,
          "sta.gw": networkGatewayIp,
          "sta.netmask": networkNetmask

        },
      }


    });
  }
  else {
    inferenceStaticData = JSON.stringify({
      config: {
        eth: {
          "ip": networkStaticIp,
          "gw": networkGatewayIp,
          "netmask": networkNetmask

        },
      }


    });
  }


  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
  fetch("/rpc/Config.Set", {
    method: "POST",
    headers: {
      "Accept": "application/json, text/plain, */*",
      "Content-type": "application/json"

    },
    body: inferenceStaticData
  })
    .then((res) => {
      if (res.status === 200) {
        return res.json()
      }
      else if (res.status === 401) {
        window.location.href = "./login.html"
      }
      else {
        alert("something went wrong");
      }
    })
    .then((data) => {

      let new_data = JSON.stringify({ reboot: false })
      fetch("/rpc/Config.Save", {
        method: "POST",
        headers: {
          "Accept": "application/json, text/plain, */*",
          "Content-type": "application/json"
        },
        body: new_data

      })
        .then((res) => {
          if (res.status === 200) {
            return res.json()
          }
          else if (res.status === 401) {
            window.location.href = "./login.html"
          }
          else {
            alert("something went wrong");
          }
        })
        .then((data) => {
    
          alert("Static Ip  parameter is set sucessfully");
        })
        .catch(err => console.log(err))

    })
    .catch(err => console.log(err))


}







document.getElementById("ntp-form").addEventListener("submit", getNtpForm)


function getNtpForm(e) {
  e.preventDefault()
  let ntpValue = document.getElementById("ntp-data").value

  var ntpDataValue = JSON.stringify({
    config: {
      sntp: {
        "server": ntpValue
      }
    }



  });

  // let BearerCheck = JSON.parse(localStorage.getItem("token") || null)
  fetch("/rpc/Config.Set", {
    method: "POST",
    headers: {
      "Accept": "application/json, text/plain, */*",
      "Content-type": "application/json",
      // Authorization: `Bearer ${BearerCheck}`,
    },
    body: ntpDataValue
  })
    .then((res) => {
      if (res.status === 200) {
        return res.json()
      }
      else if (res.status === 401) {
        window.location.href = "./login.html"
      }
      else {
        alert("something went wrong");
      }
    })
    .then((data) => {


      let new_data = JSON.stringify({ reboot: false })
      fetch("/rpc/Config.Save", {
        method: "POST",
        headers: {
          "Accept": "application/json, text/plain, */*",
          "Content-type": "application/json"
        },
        body: new_data

      })
        .then((res) => {
          if (res.status === 200) {
            return res.json()
          }
          else if (res.status === 401) {
            window.location.href = "./login.html"
          }
          else {
            alert("something went wrong");
          }
        })
        .then((data) => {
   
          alert("NTP parameter is set sucessfully");
        })
        .catch(err => console.log(err))
    })
    .catch(err => console.log(err))


}









