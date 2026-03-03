document.getElementById("location-form").addEventListener("submit", async function(event) {
    event.preventDefault();

    const key = "7d9a6d80d49b4c23b97188f5e759cfb7";
    const search = document.getElementById("location-search").value;
    const url = "https://api.geoapify.com/v1/geocode/search"

    try {
        const response = await fetch(
            `${url}?text=${encodeURIComponent(search)}&apiKey=${key}`
        );

        const data = await response.json();
        // document.getElementById("location-form-response").textContent.innerHTML = JSON.stringify(data);
        console.log(JSON.stringify(data));
    } catch (error) {
        console.log("an error occurred:", error);
    }
});
