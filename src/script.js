let worker;

const passBinaryDataToWasm = (binaryData) => {
  const uint8ArrayData = new Uint8Array(binaryData);
  worker.postMessage(
    {
      type: 'processFlac',
      binaryData: uint8ArrayData,
    },
    [uint8ArrayData.buffer]
  );
};

const modulePromise = WebAssembly.compileStreaming(
  fetch('build/process_flac.wasm')
);

try {
  if (!worker) {
    (async () => {
      worker = new Worker('src/converter_worker.js', {
        type: 'module',
        name: 'converter_worker',
      });
      worker.postMessage({
        type: 'createInstance',
        module: await modulePromise,
      });
      worker.addEventListener('message', (e) => {
        console.log(e, 'ASS');
      });
    })();
  }
} catch (error) {
  console.error('Failed to load worker:', error);
}
