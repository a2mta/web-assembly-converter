self.addEventListener('processFlac', (e) => {
    const result = yourFunction(e.data);
    self.postMessage(result);
});

function processFlac(data) {
    // Perform the computation or task here
    return data * 2; // Example: doubling the input
}