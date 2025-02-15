let api = {};
Module.onRuntimeInitialized = async () => {
  api = {
    processFlac: Module.cwrap('processFlac', null, [
      'number',
      'number',
      'number',
    ]),
    create_buffer: Module.cwrap('create_buffer', 'number', ['number']),
    free: Module.cwrap('free', null, ['number']),
  };
};

const loadFile = async () => {
  const loadFlacFile = async (url) => {
    const response = await fetch(url);
    const buffer = await response.arrayBuffer();
    return buffer;
  };

  const passBinaryDataToWasm = (binaryData) => {
    // Ensure the binary data is a Uint8Array
    const uint8ArrayData = new Uint8Array(binaryData);
    // Allocate memory in the WebAssembly module
    const dataPointer = api.create_buffer(uint8ArrayData.length);
    // Copy data to WebAssembly memory
    Module.HEAPU8.set(uint8ArrayData, dataPointer);
    console.log('SETTING');
    // Define the callback function
    const callback = addFunction((processedDataPointer, length) => {
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
    }, 'vii');
    // Call the WebAssembly function
    api.processFlac(dataPointer, uint8ArrayData.length, callback);
    // Free the allocated memory
    api.free(dataPointer);
    removeFunction(callback);
  };

  const buffer = await loadFlacFile('./input.flac');
  passBinaryDataToWasm(buffer);
};
document.getElementById('load').addEventListener('click', loadFile);
