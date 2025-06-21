let instance = null;
let globalModule = null;
self.addEventListener('message', async (e) => {
  const { module, type } = e.data;
  if (type === 'createInstance') {
    globalModule = module;
    // Instantiate the Wasm module that came via `postMessage()`.
    instance =
      instance ||
      (await WebAssembly.instantiate(module, {
        env: {
          memory: new WebAssembly.Memory({
            initial: 256,
            maximum: 512,
            shared: true,
          }),
          import: {
            proc_exit: (code) => {
              console.log(`proc_exit called with code: ${code}`);
            },
          },
          __assert_fail: () => {
            throw new Error('Assertion failed in WebAssembly');
          },
          __cxa_begin_catch: () => {},
          __cxa_end_catch: () => {},
          __cxa_find_matching_catch_2: () => {},
          __cxa_find_matching_catch_3: () => {},
          __cxa_throw: () => {
            throw new Error('C++ exception thrown in WebAssembly');
          },
          __resumeException: () => {},
          _abort_js: () => {
            throw new Error('Abort called in WebAssembly');
          },
          wasi_snapshot_preview1: {},
          _emscripten_init_main_thread_js: () => {},
          _emscripten_notify_mailbox_postmessage: () => {},
          _emscripten_receive_on_main_thread_js: () => {},
          _emscripten_thread_cleanup: () => {},
          _emscripten_thread_mailbox_await: () => {},
          _emscripten_thread_set_strongref: () => {},
          clock_time_get: () => {},
          emscripten_check_blocking_allowed: () => {},
          emscripten_exit_with_live_runtime: () => {},
          emscripten_get_now: () => {},
          emscripten_notify_memory_growth: () => {},
          proc_exit: (code) => {
            console.log(`proc_exit called with code: ${code}`);
          },
          emscripten_log: (flags, messagePtr, ...args) => {
            const memory = instance.exports.memory; // Access WASM memory
            const HEAPU8 = new Uint8Array(memory.buffer); // Create a Uint8Array view

            // Decode the message from the memory buffer
            const decoder = new TextDecoder('utf-8');
            let message = '';
            for (let i = messagePtr; HEAPU8[i] !== 0; i++) {
              message += String.fromCharCode(HEAPU8[i]);
            }

            // Format the message with additional arguments
            const formattedMessage = message.replace(/%[a-z]/g, () =>
              args.shift()
            );

            // Handle the log based on flags
            if (flags & 0x1) {
              console.log(formattedMessage);
            } else if (flags & 0x2) {
              console.warn(formattedMessage);
            } else {
              console.error(formattedMessage);
            }
          },
          emscripten_resize_heap: (size) =>
            console.log(`Heap resized to: ${size}`),
          exit: (status) => console.log(`WASM exited with status: ${status}`),
          fd_close: () => {},
          fd_read: () => {},
          fd_seek: () => {},
          fd_write: () => {},
          invoke_i: () => {},
          invoke_ii: () => {},
          invoke_iii: () => {},
          invoke_iiiiii: () => {},
          invoke_iiiiiiiiiii: () => {},
          invoke_v: () => {},
          invoke_vi: () => {},
          invoke_vii: () => {},
          invoke_viii: () => {},
          invoke_viiii: () => {},
        },
        wasi_snapshot_preview1: {
          fd_write: () => {},
          fd_read: () => {},
          fd_close: () => {},
          fd_seek: () => {},
          proc_exit: (code) => {
            console.log(`proc_exit called with code: ${code}`);
          },
        },
      }));
    self.postMessage({ result: true });
  }
  if (type === 'processFlac') {
    const { binaryData } = e.data;
    const memory = instance.exports.memory;
    const buffer = new Uint8Array(memory.buffer);

    // Allocate memory in WASM
    const dataPointer = instance.exports.malloc(binaryData.length);
    buffer.set(binaryData, dataPointer);
    console.log('Allocated dataPointer:', dataPointer, 'Length:', binaryData.length);

    // Validate length before calling processFlac
    if (typeof binaryData.length !== 'number' || binaryData.length <= 0) {
      console.error('Invalid length:', binaryData.length);
      instance.exports.free(dataPointer);
      return;
    }

    // Call processFlac with pointer and length
    instance.exports.processFlac(dataPointer, binaryData.length);

    // Free the allocated memory
    instance.exports.free(dataPointer);
    console.log('Freed dataPointer:', dataPointer);
  }
  if (type === 'createBuffer') {
    const { binaryData: uint8ArrayData } = e.data;
    console.log(uint8ArrayData.length, 'createBuffer');
    const memory = instance.exports.memory;
    const buffer = new Uint8Array(memory.buffer);

    // Allocate memory in WASM
    const dataPointer = instance.exports.malloc(+uint8ArrayData.length);
    console.log(dataPointer, 'dataPointer');
    // Copy binary data to WASM memory
    buffer.set(uint8ArrayData, dataPointer);

    // Explicitly convert length to a number before passing
    instance.exports.processFlac();

    // Optionally, read back the modified data if needed
    // const result = HEAPU8.slice(dataPointer, dataPointer + uint8ArrayData.length);

    // Send the result back to the main thread
    // self.postMessage({ type: 'processedData', result });
  }
});
