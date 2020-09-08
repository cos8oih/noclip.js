function writeMemory(module, address, bytes, callback) {
    var post = async () => {
        const response = await fetch("http://localhost:1337/write", {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json'
          },
          body: JSON.stringify(
            {
                "module" : module,
                "address" : address,
                "bytes" : bytes 
            })
        });
      }

      post()
      .then(data => {
        callback(true);
      })
      .catch((error) => {
        callback(false);
      });
}