var Module = typeof Module !== "undefined" ? Module : {};
Module.locateFile = function(path, prefix) {
  if (path === "analysis-worker.wasm") {
    return prefix + path + "?v=" + Date.now();
  }
  return prefix + path;
};

function __analysisWorkerHandleMessage(e) {
  if (!e || !e.data) {
    return;
  }

  if (e.data.type === "cancel") {
    close();
    return;
  }

  if (e.data.type !== "job" || !e.data.bytes) {
    return;
  }

  if (!self.__analysisWorkerRuntimeReady) {
    self.__analysisWorkerQueuedJob = e.data.bytes;
    return;
  }

  __analysisWorkerRunJob(e.data.bytes);
}

function __analysisWorkerRunJob(jobBytes) {
  try {
    const bytes = new Uint8Array(jobBytes);
    const malloc = Module._malloc || (typeof _malloc !== 'undefined' ? _malloc : null);
    const free = Module._free || (typeof _free !== 'undefined' ? _free : null);
    const handleFrame = Module._AnalysisWorker_HandleFrame || (typeof _AnalysisWorker_HandleFrame !== 'undefined' ? _AnalysisWorker_HandleFrame : null);
    const heap = Module.HEAPU8 || (typeof HEAPU8 !== 'undefined' ? HEAPU8 : null);

    if (!malloc || !free || !handleFrame || !heap) {
      throw new Error("WASM exports missing: malloc=" + !!malloc + " free=" + !!free + " handle=" + !!handleFrame + " heap=" + !!heap);
    }

    const ptr = malloc(bytes.length);
    if (!ptr) {
      throw new Error("_malloc returned null");
    }

    heap.set(bytes, ptr);
    handleFrame(ptr, bytes.length);
    free(ptr);
  } catch (error) {
    postMessage({
      type: "error",
      message: error && error.stack ? error.stack : String(error)
    });
    close();
  }
}

if (!self.__analysisWorkerHandlersInstalled) {
  self.__analysisWorkerHandlersInstalled = true;
  self.__analysisWorkerRuntimeReady = false;
  self.addEventListener("message", __analysisWorkerHandleMessage);
  postMessage({ type: "script-loaded" });
}

var __analysisWorkerPreviousRuntimeInitialized = Module.onRuntimeInitialized;
Module.onRuntimeInitialized = function() {
  if (typeof __analysisWorkerPreviousRuntimeInitialized === "function") {
    __analysisWorkerPreviousRuntimeInitialized();
  }

  self.__analysisWorkerRuntimeReady = true;
  postMessage({ type: "ready" });

  if (self.__analysisWorkerQueuedJob) {
    const queuedJob = self.__analysisWorkerQueuedJob;
    self.__analysisWorkerQueuedJob = null;
    __analysisWorkerRunJob(queuedJob);
  }
};
