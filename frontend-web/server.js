const express = require('express');
const bodyParser = require('body-parser');
const mysql = require('mysql');
const path = require('path');

const app = express();
const port = 3000;

app.use(bodyParser.urlencoded({ extended: true }));
app.use(express.static(path.join(__dirname, 'public')));

const db = mysql.createConnection({
  host: 'localhost',
  user: 'root',
  password: '',    // your MySQL password
  database: 'social_security_db'   // your database
});

db.connect((err) => {
  if (err) throw err;
  console.log('Connected to database.');
});

app.get('/', (req, res) => {
  res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

app.post('/dashboard', (req, res) => {
  const email = req.body.email;
  const password = req.body.password;

  const query = 'SELECT * FROM users WHERE email = ? AND password = ?';
  db.query(query, [email, password], (err, results) => {
    if (err) throw err;
    
    if (results.length > 0) {
      const user = results[0];
      res.send(`
        <html>
        <head><title>Application Status</title></head>
        <body style="font-family: Arial, sans-serif; background-color: #f4f4f9;">
          <div style="text-align: center; margin-top: 50px;">
            <h1>Welcome, ${user.email}!</h1>
            <p>Your Application Status: <strong>${user.application_status}</strong></p>
            <a href="/" style="text-decoration: none; color: #2575fc; font-size: 18px;">Logout</a>
          </div>
        </body>
        </html>
      `);
    } else {
      res.send('<h1>Login failed</h1><p>Invalid Email or Password. <a href="/">Try again</a></p>');
    }
  });
});

app.listen(port, () => {
  console.log(`Server running on http://localhost:${port}`);
});
