import argon2 from 'argon2';
import crypto from 'crypto';
import mysql from 'mysql2/promise';
import express from 'express';
const server = express();
const port = 3080;
const dbmsIP = '192.168.228.128';

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
    console.log('Password verification success:', await verifyHash('somePassword', hash));
});

// Start the server
server.get('/', (req, res) => {
	res.send('Hello World!');
});

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

try {
	const [results, fields] = await dbConnection.execute(
		'SELECT * FROM users'
	);
	console.log(results);
	console.log(fields);
} catch (err) {
	console.error(err);
}



