let callback;
let api = {};
let worker;
const bar = document.getElementById('bar');
const loadFlacFile = async (url) => {
  const response = await fetch(url);
  const buffer = await response.arrayBuffer();
  return buffer;
};

const callBackFunction = (processedDataPointer, length) => {
  console.log('CALLBACK');

  const processedData = Module.HEAPU8.subarray(
    processedDataPointer,
    processedDataPointer + length
  ).slice().buffer;

  const blob = new Blob([processedData], { type: 'audio/mpeg' });
  console.log('Processed data:', processedData, blob);
  const url = URL.createObjectURL(blob);
  const a = document.createElement('a');
  a.href = url;
  a.download = 'output.mp3';
  document.body.appendChild(a);
  a.click();
  document.body.removeChild(a);
  URL.revokeObjectURL(url);
};

Module.onRuntimeInitialized = async () => {
  callback = addFunction(callBackFunction, 'vii');
  api = {
    processFlac: Module.cwrap(
      'processFlac',
      null,
      ['number', 'number', 'number', 'number'],
      { async: true }
    ), // Add async option
    create_buffer: Module.cwrap('create_buffer', 'number', ['number']),
    free: Module.cwrap('free', null, ['number']),
  };
};

const passBinaryDataToWasm = (binaryData) => {
  // Ensure the binary data is a Uint8Array
  const uint8ArrayData = new Uint8Array(binaryData);
  // Allocate memory in the WebAssembly module
  const dataPointer = api.create_buffer(uint8ArrayData.length);
  // Copy data to WebAssembly memory
  Module.HEAPU8.set(uint8ArrayData, dataPointer);

  const start = new Date();
  console.log('Start processing', start);

  // Create the worker if it doesn't exist
  if (!worker) {
    worker = new Worker('process_worker.js');
  }

  console.log(worker);

  // Pass the worker handle and data to the processFlac function
  api.processFlac(worker, dataPointer, uint8ArrayData.length, callback);

  console.log('End processing', new Date() - start);
};

// Define the progress callback
Module.onProgress = (percentage) => {
  bar.style.width = `${Math.round(percentage)}%`;
  console.log(`Processing: ${percentage.toFixed(2)}%`);
};
