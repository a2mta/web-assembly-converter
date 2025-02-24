let callback;
let api = {};
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
  );
  console.log('Processed data:', processedData);
  const blob = new Blob([processedData], { type: 'audio/mpeg' });
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
      ['number', 'number', 'number'],
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
  api.processFlac(dataPointer, uint8ArrayData.length, callback);
  console.log('End processing', new Date() - start);
  // .then(() => {
  //   // Free the allocated memory
  //   api.free(dataPointer);
  //   removeFunction(callback);
  // })
  // .catch((err) => {
  //   console.error('Error processing FLAC:', err);
  //   api.free(dataPointer);
  //   removeFunction(callback);
  // });

  //   const processChunk = (start) => {
  //     const chunkSize = 1024 * 1024; // 1MB chunks
  //     const end = Math.min(start + chunkSize, uint8ArrayData.length);
  //     const chunk = uint8ArrayData.subarray(start, end);
  //     Module.HEAPU8.set(chunk, dataPointer + start);

  //     if (end < uint8ArrayData.length) {
  //       setTimeout(() => processChunk(end), 0);
  //     } else {
  //       // Call the WebAssembly function
  //       api
  //         .processFlac(dataPointer, uint8ArrayData.length, callback)
  //         .then(() => {
  //           // Free the allocated memory
  //           api.free(dataPointer);
  //           removeFunction(callback);
  //         })
  //         .catch((err) => {
  //           console.error('Error processing FLAC:', err);
  //           api.free(dataPointer);
  //           removeFunction(callback);
  //         });
  //     }
  //   };

  //   processChunk(0);
};

// Define the progress callback
Module.onProgress = (percentage) => {
  bar.style.width = `${Math.round(percentage)}%`;
  console.log(`Processing: ${percentage.toFixed(2)}%`);
};

const loadFile = async () => {
  const buffer = await loadFlacFile('./input.flac');
  passBinaryDataToWasm(buffer);
};

document.getElementById('load').addEventListener('click', loadFile);
