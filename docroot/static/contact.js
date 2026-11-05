// Step 1 + 4: prevent default submit, POST with fetch, show confirmation.
// Uses event delegation because Wt creates the form dynamically.
document.addEventListener('submit', function (e) {
  var form = e.target;
  if (!form || form.id !== 'contact-form') return;
  e.preventDefault();                       // stop normal form submission

  var status = document.getElementById('form-status');
  function show(msg, type) { status.textContent = msg; status.className = 'status ' + type; }

  var payload = {
    name: document.getElementById('cf-name').value.trim(),
    email: document.getElementById('cf-email').value.trim(),
    message: document.getElementById('cf-message').value.trim()
  };

  if (!payload.name || !payload.message || payload.email.indexOf('@') === -1) {
    show('Please fill in a valid name, email and message.', 'error');
    return;
  }

  show('Sending...', 'pending');

  fetch('/api/contact', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload)
  })
    .then(function (res) {
      return res.json().then(function (data) { return { ok: res.ok, data: data }; });
    })
    .then(function (r) {
      if (r.ok && r.data.success) {          // Step 4: confirmation message
        show(r.data.message, 'success');
        form.reset();
      } else {
        show(r.data.error || 'Something went wrong.', 'error');
      }
    })
    .catch(function () { show('Could not reach the server.', 'error'); });
}, true);
