import os
import json
import uuid
import subprocess
import shutil
from fastapi import FastAPI, File, UploadFile, Form, HTTPException
from fastapi.responses import FileResponse, JSONResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware

app = FastAPI(title="SCAD-TSA API", description="Backend API for Trie-based Anomaly Detection")

# Enable CORS for frontend integration
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Directory configurations
WEB_DIR = os.path.dirname(os.path.abspath(__file__))
ROOT_DIR = os.path.dirname(WEB_DIR)
UPLOADS_DIR = os.path.join(WEB_DIR, "uploads")
RESULTS_DIR = os.path.join(WEB_DIR, "results")

# Ensure directories exist
os.makedirs(UPLOADS_DIR, exist_ok=True)
os.makedirs(RESULTS_DIR, exist_ok=True)

# Mount results directory to serve generated PNG files
app.mount("/results", StaticFiles(directory=RESULTS_DIR), name="results")

# Serve assets if the directory exists
ASSETS_DIR = os.path.join(WEB_DIR, "assets")
if os.path.exists(ASSETS_DIR):
    app.mount("/assets", StaticFiles(directory=ASSETS_DIR), name="assets")

@app.get("/")
async def read_index():
    index_path = os.path.join(WEB_DIR, "index.html")
    if os.path.exists(index_path):
        return FileResponse(index_path)
    return {"message": "Welcome to SCAD-TSA API. Frontend index.html not found."}

@app.get("/style.css")
async def get_style():
    style_path = os.path.join(WEB_DIR, "style.css")
    if os.path.exists(style_path):
        return FileResponse(style_path)
    return HTTPException(status_code=404, detail="style.css not found")

@app.get("/script.js")
async def get_script():
    script_path = os.path.join(WEB_DIR, "script.js")
    if os.path.exists(script_path):
        return FileResponse(script_path)
    return HTTPException(status_code=404, detail="script.js not found")

@app.post("/api/analyze")
async def analyze(
    uploadFile: UploadFile = File(...),
    config: str = Form(...)
):
    # 1. Parse configuration JSON
    try:
        cfg = json.loads(config)
    except Exception as e:
        raise HTTPException(status_code=400, detail=f"Invalid config JSON: {str(e)}")

    # Generate a unique task ID for this analysis run
    task_id = str(uuid.uuid4())
    task_upload_dir = os.path.join(UPLOADS_DIR, task_id)
    task_result_dir = os.path.join(RESULTS_DIR, task_id)

    os.makedirs(task_upload_dir, exist_ok=True)
    os.makedirs(task_result_dir, exist_ok=True)

    # 2. Save uploaded file
    input_file_path = os.path.join(task_upload_dir, uploadFile.filename)
    try:
        with open(input_file_path, "wb") as buffer:
            shutil.copyfileobj(uploadFile.file, buffer)
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to save uploaded file: {str(e)}")

    # 3. Build command arguments for main_pipeline
    # Select executable name based on OS (.exe on Windows, extensionless on others)
    exe_name = "main_pipeline.exe" if os.name == "nt" else "main_pipeline"
    executable_path = os.path.join(ROOT_DIR, exe_name)

    if not os.path.exists(executable_path):
        raise HTTPException(
            status_code=500,
            detail=f"Pipeline executable not found at '{executable_path}'. Please compile the C++ source files first."
        )

    # Command: executable <input_file> <output_dir> [flags]
    cmd = [executable_path, input_file_path, task_result_dir]

    # Map preprocessing configuration
    delim = cfg.get("delim")
    ignore = cfg.get("ignore")
    regex = cfg.get("regex")

    if regex:
        cmd.append(f"--regex={regex}")
    else:
        if delim:
            cmd.append(f"--delim={delim}")
        if ignore:
            cmd.append(f"--ignore={ignore}")

    # Map analysis thresholds (percentiles)
    freq_perc = cfg.get("frequencyPerc")
    len_perc = cfg.get("lengthPerc")
    entropy_perc = cfg.get("entropyPerc")

    if freq_perc is not None:
        cmd.append(f"--perc-freq={freq_perc}")
    if len_perc is not None:
        cmd.append(f"--perc-len={len_perc}")
    if entropy_perc is not None:
        cmd.append(f"--perc-entropy={entropy_perc}")

    # Map visualization choice
    visual = cfg.get("visual", "none")
    if visual == "complete":
        cmd.append("--visual-complete")
    elif visual == "partial":
        cmd.append("--visual-partial")
    elif visual == "freq":
        cmd.append("--visual-freq")
    elif visual == "len":
        cmd.append("--visual-len")
    elif visual == "entropy":
        cmd.append("--visual-entropy")

    # 4. Execute the C++ pipeline
    try:
        # Run subprocess with working directory set to project root to allow internal binaries resolve correctly
        result = subprocess.run(
            cmd,
            cwd=ROOT_DIR,
            capture_output=True,
            text=True,
            check=False
        )
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Failed to execute pipeline: {str(e)}")

    if result.returncode != 0:
        # Pipeline execution failed
        error_msg = result.stderr if result.stderr else result.stdout
        raise HTTPException(
            status_code=500,
            detail=f"Analysis pipeline error (exit code {result.returncode}): {error_msg}"
        )

    # 5. Read outputs
    table_data = {}
    csv_mappings = {
        "allEntries": "all_entries.csv",
        "freqAnomalies": "frequency_anomalies.csv",
        "lenAnomalies": "length_anomalies.csv",
        "entropyAnomalies": "entropy_anomalies.csv"
    }

    for key, filename in csv_mappings.items():
        file_path = os.path.join(task_result_dir, filename)
        if os.path.exists(file_path):
            try:
                with open(file_path, "r", encoding="utf-8") as f:
                    table_data[key] = f.read()
            except Exception as e:
                table_data[key] = f"[ERROR] Failed to read {filename}: {str(e)}"
        else:
            table_data[key] = ""

    # 6. Check for generated visualization images
    visuals = {}
    png_mappings = {
        "complete": "complete_trie.png",
        "partial": "partial_trie.png",
        "freq": "frequency_anomalies.png",
        "len": "length_anomalies.png",
        "entropy": "entropy_anomalies.png"
    }

    for key, filename in png_mappings.items():
        file_path = os.path.join(task_result_dir, filename)
        if os.path.exists(file_path):
            visuals[key] = f"/results/{task_id}/{filename}"
        else:
            visuals[key] = None

    return JSONResponse(
        status_code=200,
        content={
            "status": "success",
            "message": "Analysis completed successfully",
            "tableData": table_data,
            "visuals": visuals,
            "taskId": task_id
        }
    )

if __name__ == "__main__":
    import uvicorn
    # Start the server on port 8000
    uvicorn.run("app:app", host="127.0.0.1", port=8000, reload=True)
