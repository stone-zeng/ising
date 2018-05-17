$SETTINGS_FILE_NAME = "ising-settings-" + (get-date).ToString("yyyyMMddThhmmss") + ".json"
Out-File -FilePath $SETTINGS_FILE_NAME -Encoding ascii -InputObject `
"{
    `"size.list`": [40],
    `"boundary`": `"periodic`",
    `"temperature.span`": {
        `"begin`": 1.0,
        `"end`": 3.5,
        `"step`": 0.1
    },
    `"externalMagneticField.list`": [0.2],
    `"iterations`": 2000,
    `"repetitions`": 10
}"
