const dropZone = document.querySelector('.dropzone');
dropZone.addEventListener('dragover', (event) => {
  event.preventDefault();
  dropZone.style.borderColor = '#000';
});

dropZone.addEventListener('dragleave', () => {
  dropZone.style.borderColor = '#aaa';
});

dropZone.addEventListener('drop', (event) => {
  event.preventDefault();
  dropZone.style.borderColor = '#aaa';

  const files = event.dataTransfer.files;
  for (const file of files) {
    const reader = new FileReader();
    reader.onload = (event) => {
      const fileData = event.target.result;
      console.log(fileData);
      passBinaryDataToWasm(fileData);
    };
    reader.readAsArrayBuffer(file)
  }
});
