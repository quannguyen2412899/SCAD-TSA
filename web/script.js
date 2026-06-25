// Main JavaScript Control Logic for SCAD-TSA Interface

const fileInput = document.getElementById("input-file");
const dropzone = document.getElementById("dropzone");
const fileBadge = document.getElementById("file-badge");
const fileNameText = document.getElementById("file-name-text");
const configCard = document.getElementById("config-card");
const runBtn = document.getElementById("run");
const btnSpinner = document.getElementById("btn-spinner");
const resultsCard = document.getElementById("results");

// 1. Drag and Drop Interaction
['dragenter', 'dragover'].forEach(eventName => {
    dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        dropzone.classList.add('dragover');
    }, false);
});

['dragleave', 'drop'].forEach(eventName => {
    dropzone.addEventListener(eventName, (e) => {
        e.preventDefault();
        dropzone.classList.remove('dragover');
    }, false);
});

dropzone.addEventListener('drop', (e) => {
    const dt = e.dataTransfer;
    const files = dt.files;
    if (files.length > 0) {
        fileInput.files = files;
        handleFileSelection(files[0]);
    }
});

fileInput.addEventListener("change", function() {
    if (this.files.length > 0) {
        handleFileSelection(this.files[0]);
    }
});

function handleFileSelection(file) {
    fileNameText.textContent = file.name;
    fileBadge.style.display = "flex";
    configCard.style.display = "block";
    
    // Smooth scroll down to configuration panel
    configCard.scrollIntoView({ behavior: 'smooth' });
}

// 2. Tab Navigation
function openTab(boxId, button) {
    const boxes = document.querySelectorAll('.result-box');
    boxes.forEach(box => {
        box.classList.remove('active');
    });

    const buttons = document.querySelectorAll('.tab-button');
    buttons.forEach(btn => {
        btn.classList.remove('active');
    });

    document.getElementById(boxId).classList.add('active');
    button.classList.add('active');
}

// 3. API Communication
async function sendRequest() {
    const file = fileInput.files[0];
    if (!file) {
        alert("Vui lòng tải lên file dữ liệu trước!");
        return;
    }

    // Toggle loading states
    runBtn.disabled = true;
    btnSpinner.style.display = "inline-block";
    runBtn.querySelector('.btn-text').textContent = "Đang phân tích...";

    // Gather inputs
    const delim = document.getElementById("delim").value;
    const ignore = document.getElementById("ignore").value;
    const regex = document.getElementById("regex").value;
    const freqPerc = document.getElementById("freq-perc").valueAsNumber;
    const lenPerc = document.getElementById("len-perc").valueAsNumber;
    const entropyPerc = document.getElementById("entropy-perc").valueAsNumber;
    const visual = document.getElementById("visual-select").value;

    const configuration = {
        delim: delim,
        ignore: ignore,
        regex: regex,
        frequencyPerc: isNaN(freqPerc) ? 5.0 : freqPerc,
        lengthPerc: isNaN(lenPerc) ? 5.0 : lenPerc,
        entropyPerc: isNaN(entropyPerc) ? 95.0 : entropyPerc,
        visual: visual
    };

    const formData = new FormData();
    formData.append("uploadFile", file);
    formData.append("config", JSON.stringify(configuration));

    try {
        const response = await fetch('/api/analyze', {
            method: 'POST',
            body: formData
        });

        await handleResponse(response);
    } catch (error) {
        console.error(error);
        alert("Lỗi kết nối đến server: " + error.message);
    } finally {
        // Reset loading states
        runBtn.disabled = false;
        btnSpinner.style.display = "none";
        runBtn.querySelector('.btn-text').textContent = "Bắt đầu phân tích";
    }
}

// 4. Handle API Response
async function handleResponse(response) {
    if (!response.ok) {
        const errData = await response.json().catch(() => ({}));
        alert("Đã xảy ra lỗi khi phân tích: " + (errData.detail || response.statusText));
        return;
    }

    const data = await response.json();
    
    // Show results panel
    resultsCard.style.display = "block";
    document.getElementById("task-id-text").textContent = data.taskId;

    const tables = data.tableData || {};
    
    // Insert CSV data into respective tables
    insertTable("table-freq", tables.freqAnomalies, "Không phát hiện thấy bất thường tần suất.");
    insertTable("table-len", tables.lenAnomalies, "Không phát hiện thấy bất thường độ dài.");
    insertTable("table-entropy", tables.entropyAnomalies, "Không phát hiện thấy bất thường Entropy.");
    insertTable("table-all", tables.allEntries, "Không có dữ liệu.");

    // Handle visuals (render Trie diagrams if generated)
    const visuals = data.visuals || {};
    const visualMapping = {
        "box-freq": "freq",
        "box-len": "len",
        "box-entropy": "entropy",
        "box-all": "complete" // for all entries we can show complete trie if selected
    };

    // If 'partial' is selected, display it in the active tab (freq, len or entropy) depending on mapping
    const isPartialSelected = document.getElementById("visual-select").value === "partial";

    for (const [boxId, key] of Object.entries(visualMapping)) {
        const boxDiv = document.getElementById(boxId);
        
        // Remove old visual container if it exists
        const oldVisual = boxDiv.querySelector('.visual-container');
        if (oldVisual) oldVisual.remove();

        // Check if there is an image or JSON to display
        let visualObj = visuals[key];
        if ((!visualObj || (!visualObj.json && !visualObj.png)) && isPartialSelected && visuals["partial"] && key !== "complete") {
            // Fallback: if partial trie is selected, render it for Freq, Len, Entropy tabs
            visualObj = visuals["partial"];
        }

        if (visualObj && (visualObj.json || visualObj.png)) {
            const visualContainer = document.createElement("div");
            visualContainer.className = "visual-container";
            
            let downloadButtonsHtml = `<div class="download-buttons-group">`;
            if (visualObj.json) {
                downloadButtonsHtml += `
                    <a href="${visualObj.json}" download class="btn-download btn-download-json">
                        📥 Tải file JSON cấu trúc Trie
                    </a>
                `;
            }
            if (visualObj.png) {
                downloadButtonsHtml += `
                    <a href="${visualObj.png}" download class="btn-download btn-download-png">
                        🖼️ Tải ảnh Trie (PNG)
                    </a>
                `;
            }
            downloadButtonsHtml += `</div>`;

            let imgHtml = "";
            if (visualObj.png) {
                imgHtml = `
                    <div class="visual-img-wrapper">
                        <img src="${visualObj.png}" alt="Trie Diagram" class="trie-image" onclick="openImageModal('${visualObj.png}')" />
                    </div>
                    <p class="img-hint">Nhấp vào hình ảnh để phóng to toàn màn hình</p>
                `;
            } else if (visualObj.json) {
                // JSON exists but PNG is missing (missing graphviz system tool)
                imgHtml = `
                    <div class="visual-warning-wrapper">
                        <span class="warning-icon">⚠️</span>
                        <div class="warning-content">
                            <p class="warning-title">Không thể xuất ảnh sơ đồ cây Trie</p>
                            <p class="warning-text">
                                <strong>Lý do:</strong> Máy chủ chưa cài đặt phần mềm hệ thống <strong>Graphviz</strong> (lệnh <code>dot</code>) hoặc chưa cấu hình biến môi trường PATH.<br>
                                Tuy nhiên, bạn vẫn có thể tải về file cấu trúc dữ liệu JSON ở trên.
                            </p>
                            <p class="warning-help">
                                <strong>Để hiển thị hình vẽ:</strong> Hãy cài đặt Graphviz từ <a href="https://graphviz.org/" target="_blank">graphviz.org</a> và thêm thư mục chứa lệnh <code>dot.exe</code> (thường là <code>C:\\Program Files\\Graphviz\\bin</code>) vào biến môi trường <strong>PATH</strong> của hệ thống, sau đó khởi động lại server.
                            </p>
                        </div>
                    </div>
                `;
            }

            visualContainer.innerHTML = `
                <h3>📁 File Cấu Trúc Cây Trie</h3>
                ${downloadButtonsHtml}
                ${imgHtml}
            `;
            boxDiv.appendChild(visualContainer);
        }
    }

    // Scroll to results
    resultsCard.scrollIntoView({ behavior: 'smooth' });
}

// 5. Generate HTML Tables from CSV
function insertTable(divId, csvString, emptyMessage) {
    const container = document.getElementById(divId);
    
    if (!csvString || csvString.trim() === "") {
        container.innerHTML = `<div class="empty-state"><p>${emptyMessage}</p></div>`;
        return;
    }

    const parsed = Papa.parse(csvString, {
        header: false,
        skipEmptyLines: true,
        dynamicTyping: false
    });

    const rows = parsed.data;

    if (!rows || rows.length === 0) {
        container.innerHTML = `<div class="empty-state"><p>${emptyMessage}</p></div>`;
        return;
    }

    let html = "<table><thead><tr>";
    
    // Table Header
    rows[0].forEach(cell => {
        html += `<th>${cell}</th>`;
    });
    html += "</tr></thead><tbody>";

    // Table Body
    for (let i = 1; i < rows.length; i++) {
        // Check if row indicates an anomaly
        const isAnomaly = rows[i].some(cell => typeof cell === 'string' && cell.toLowerCase().includes('anomaly') || cell.toLowerCase().includes('entropy') || cell.toLowerCase().includes('frequency') || cell.toLowerCase().includes('length'));
        const rowClass = isAnomaly ? 'class="anomaly-row"' : '';
        
        html += `<tr ${rowClass}>`;
        rows[i].forEach((cell, cellIndex) => {
            let cellContent = cell;
            let cellClass = "";

            // Format Anomaly tags
            if (cellIndex === rows[i].length - 1 && cell && cell.trim() !== "") {
                cellClass = 'class="anomaly-badge"';
            }

            // Format Rate (percentage index)
            if (cellIndex === rows[i].length - 2 && !isNaN(parseFloat(cell))) {
                const val = parseFloat(cell);
                const badgeColor = val < 0.2 ? 'rate-warning' : 'rate-normal';
                cellContent = `<span class="rate-badge ${badgeColor}">${(val * 100).toFixed(1)}%</span>`;
            }

            html += `<td ${cellClass}>${cellContent}</td>`;
        });
        html += "</tr>";
    }
    
    html += "</tbody></table>";
    container.innerHTML = html;
}

// 6. Image Lightbox Modal Functions
const modal = document.getElementById("image-modal");
const modalImg = document.getElementById("modal-img");

function openImageModal(imgUrl) {
    modal.style.display = "block";
    modalImg.src = imgUrl;
}

function closeImageModal() {
    modal.style.display = "none";
}

// Close modal when pressing Escape key
document.addEventListener('keydown', function(event) {
    if (event.key === "Escape") {
        closeImageModal();
    }
});