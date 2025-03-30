import argon2 from 'argon2';
import crypto from 'crypto';
import mysql from 'mysql2/promise';
import express from 'express';
import 'dotenv/config'
const server = express();
server.use(express.json());
const port = 3080;
const dbmsIP = process.env.DBMS_IP;

const hashOptions = {
	parallelism: 1,
	memoryCost: 32000,
	timeCost: 3,
	keylength: 255,
}

async function hashString(str: string): Promise<string> {
	const salt = crypto.randomBytes(16);
	return await argon2.hash(str, {
		...hashOptions,
		salt,
	});
}

async function verifyHash(str: string, hash: string) {
	return await argon2.verify(hash, str);
}

hashString('somePassword').then(async (hash) => {
    console.log('Hash + salt of the password:', hash)
    console.log('Password verification success:', await verifyHash('somePasswordhetd', hash));
});

server.get('/', (req, res) => {
	res.send('Hello World!');
});
server.post('/login', async (req, res) => {
	res.send('Logged In!');
	console.log(req.body);
	req.body
	try {
		const [results, fields] = await dbConnection.execute(
			'SELECT * FROM `users` WHERE `email` = ?',
			[req.body['email']]
		)
		console.log(results);
		console.log(fields);
	} catch (err) {
		console.error(err);
	}
});
server.post('/login', async (req, res) => {
	res.send('Logged In!');
	console.log(req.body);
	req.body
	try {
		const [results, fields] = await dbConnection.execute(
			'SELECT * FROM `users` WHERE `email` = ?',
			[req.body['email']]
		)
		console.log(results);
		console.log(fields);
	} catch (err) {
		console.error(err);
	}
});
server.post('/register', async (req, res) => {
	res.send('Register request...');
	console.log(req.body);
	req.body
	try {
		const [results, fields] = await dbConnection.execute(
			'SELECT * FROM `users` WHERE `email` = ?',
			[req.body['email']]
		)
		console.log(results);
		console.log(fields);
	} catch (err) {
		console.error(err);
	}
});

// Start the server
server.listen(port, () => {
	console.log(`Server listening at http://localhost:${port}`);
});

// Establish connection to MySQL database
const dbConnection = await mysql.createConnection({
	host: dbmsIP,
	user: 'bss',
	password: 'bss25',
	database: 'application_system'
});

