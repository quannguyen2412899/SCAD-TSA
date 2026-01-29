const fileInput = document.getElementById("input-file");

fileInput.addEventListener("change", function() {
    if (this.files.length > 0) {
        document.getElementById("config").style.display = "block";
        document.getElementById("run").style.display = "flex";
        // document.getElementById("results").style.display = "block";
    }
    else {
        document.getElementById("config").style.display = "none";
        document.getElementById("run").style.display = "none";
        document.getElementById("results").style.display = "none";
    }
});

async function sendRequest() {
    const file = fileInput.files[0];
    if (file) {
        const delimiters = document.getElementById("delim").value;
        const ignoreChars = document.getElementById("ignore").value;
        const regularExpr = document.getElementById("regex").value;
        const freqPerc = document.getElementById("freq-perc").valueAsNumber;
        const lenPerc = document.getElementById("len-perc").valueAsNumber;
        const enPerc = document.getElementById("entropy-perc").valueAsNumber;

        const configuration = {
            delim: delimiters,
            ignore: ignoreChars,
            regex: regularExpr,
            frequencyPerc: freqPerc,
            lengthPerc: lenPerc,
            entropyPerc: enPerc
        }
        const jsonConfig = JSON.stringify(configuration);

        // console.log("Config: " + jsonConfig);

        const formData = new FormData();
        formData.append("uploadFile", file);
        formData.append("config", jsonConfig);

        const response = await fetch('/api/analyze', {
            method: 'POST',
            body: formData
        });

        handleResposne(response);
    }

    else {
        alert("Upload a file to start analysis!");
    }
}

function handleResposne(response) {
    // ***
}

function openTab(box, button) {
    const freqResId = "box-freq";
    const lenResId = "box-len";
    const entropyResId = "box-entropy";
    const freqRes = document.getElementById(freqResId);
    const lenRes = document.getElementById(lenResId);
    const entropyRes = document.getElementById(entropyResId);

    document.querySelectorAll('.tab-button').forEach(function(button_) {
        button_.classList.remove("active");
    });
    freqRes.style.display = "none";
    lenRes.style.display = "none";
    entropyRes.style.display = "none";

    button.classList.add("active");
    if (box == freqResId) {
        freqRes.style.display = "block";
    }
    else if (box == lenResId) {
        lenRes.style.display = "block";
    }
    else if (box == entropyResId) {
        entropyRes.style.display = "block";
    }
}