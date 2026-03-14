const connectBtn = document.getElementById('connectBtn');
const viewerName = document.getElementById('viewerName');
const streamName = document.getElementById('streamName');
const jsonFeed = document.getElementById('jsonFeed');

connectBtn.addEventListener('click', () => {
  const ws = new WebSocket('ws://localhost:9001');
  ws.onopen = () => {
    jsonFeed.textContent = `Connected as ${viewerName.value || 'viewer'} for ${streamName.value}\n`;
  };
  ws.onmessage = (event) => {
    jsonFeed.textContent += `${event.data}\n`;
  };
  ws.onerror = () => {
    jsonFeed.textContent += 'WebSocket error\n';
  };
});
