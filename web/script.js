async function sendRequest() {

    const file = document.getElementById("input-file").files[0];

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

        const formData = new FormData();
        formData.append("uploadFile", file);
        formData.append("config", jsonConfig);

        const response = await fetch('/api/analyze', {
            method: 'POST',
            body: formData
        });
    }

    else {
        alert("Upload a file to start analysis!");
    }
}