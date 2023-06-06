const socket = new WebSocket(`ws://${window.location.hostname}/`);

const message = document.getElementById('message-input');
const sendMsg = document.getElementById('send-message');
const msgSound = document.getElementById('notification-sound');
const user = document.getElementById('username-input');
const sendUser = document.getElementById('send-username');
const displayMsg = document.getElementById('display-message');
const typingLabel = document.getElementById('typing-label');
const chatWindow = document.getElementById('chat-window');
const usersCounter = document.getElementById('users-counter');
const msgErr = document.getElementById('message-error');
const userErr = document.getElementById('username-error');
const join = document.getElementById('you-joined');
const chat = document.getElementById('chat');
const login = document.getElementById('login-page');

sendUser.addEventListener('click', () => {
  if (user.value === null || user.value.trim().length === 0) {
    userErr.innerHTML = '🚨 Name is required!';
    return;
  }

  userErr.innerHTML = '';
  login.style.display = 'none';
  chat.style.display = 'block';
  join.innerHTML = '<p>You have joined the chat!<p>';
  socket.send(JSON.stringify({ type: 'new-user', data1: user.value }));
});

sendMsg.addEventListener('click', () => {
  if (message.value === null || message.value.trim().length === 0) {
    msgErr.innerHTML = '🚨 Message is required!';
    return;
  }

  socket.send(
    JSON.stringify({
      type: 'new-message',
      data1: user.value,
      data2: message.value,
    })
  );
  message.value = '';
  msgErr.innerHTML = '';
});

message.addEventListener('keypress', () => {
  socket.send(JSON.stringify({ type: 'is-typing', data1: user.value }));
});

socket.addEventListener('open', () => {
  console.log('Connected to server');
});

socket.addEventListener('message', (event) => {
  const data = JSON.parse(event.data);
  const { type, data1, data2 } = data;

  switch (type) {
    case 'user-connected':
      displayMsg.innerHTML += `<p><strong>${data1}</strong> has connected!</p>`;
      chatWindow.scrollTop = chatWindow.scrollHeight;
      msgSound.play();
      break;

    case 'broadcast':
      usersCounter.innerHTML = data1;
      break;

    case 'new-message':
      typingLabel.innerHTML = '';
      displayMsg.innerHTML += `<p><strong>${data1}</strong><em> at ${new Date().getHours()}:${new Date().getMinutes()}</em> : ${data2}</p>`;
      chatWindow.scrollTop = chatWindow.scrollHeight;
      msgSound.play();
      break;

    case 'is-typing':
      typingLabel.innerHTML = `<p>${data1} is typing...</p>`;
      chatWindow.scrollTop = chatWindow.scrollHeight;
      break;

    case 'user-disconnected':
      if (data1 == null) {
        displayMsg.innerHTML += '<p>Unlogged user has disconnected!</p>';
        chatWindow.scrollTop = chatWindow.scrollHeight;
      } else {
        displayMsg.innerHTML += `<p><strong>${data1}</strong> has disconnected!</p>`;
        chatWindow.scrollTop = chatWindow.scrollHeight;
        msgSound.play();
      }
      break;
  }
});

socket.addEventListener('close', () => {
  console.log('Disconnected from server');
});

socket.addEventListener('error', (error) => {
  console.error('WebSocket error:', error);
});
