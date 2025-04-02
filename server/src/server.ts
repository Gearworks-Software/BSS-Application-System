import argon2 from 'argon2';
import crypto from 'crypto';
import mysql, { QueryError, QueryResult, ResultSetHeader } from 'mysql2/promise';
import express, { Request, Response } from 'express';
import router from './routers/apiRouter'
import 'dotenv/config'

const server = express();
server.use(express.json());
// server.use(router)
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
	return await argon2.verify(str, hash);
}

server.get('/', (req, res) => {
	res.send('Hello World!');
});

server.post('/login', async (req: Request, res: Response) => {
	console.log('LOGIN REQUEST')
	try {
		// Check if body contains all required properties
		const { email, password } = req.body;
		if ( !email || !password) {
			return res.status(400).send('Malformed login request');
		}

		// Check if record with email exists
		const [rows]: [mysql.RowDataPacket[], mysql.FieldPacket[]] = await dbPool.execute(
			'SELECT * FROM `internal_users` WHERE `email` = ?',
			[email]
		);
		if (rows.length === 0) {
			return res.status(401).send('Incorrect username or password');
		}

		// Check if password is correct
		const passwordMatches = await verifyHash(rows[0].password, password);
		if (!passwordMatches) {
			return res.status(401).send('Incorrect username or password');
		}

		return res.status(200).send('Logged in successfully');

	} catch (err) {
		console.error('(/login Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});
server.post('/register', async (req: Request, res: Response) => {
	console.log('REGISTER REQUEST')
	try {
		// Check if body contains all required properties
		const { fName, lName, role, email, password } = req.body;
		console.debug(fName, lName, role, email, password);
		if ( !fName || !lName || !role || !email || !password ) {
			return res.status(400).send('Malformed register request');
		}

		// Check if user already exists
		const [rows]: [mysql.RowDataPacket[], mysql.FieldPacket[]] = await dbPool.execute(
			'SELECT * FROM `internal_users` WHERE `email` = ?',
			[email]
		);
		if (rows.length > 0) {
			return res.status(409).send('User already exists');
		}

		// Hash password
		const hashedPassword = await hashString(password);

		// Add new user to database
		const [inserted] = await dbPool.execute<ResultSetHeader>(
			'INSERT INTO `internal_users` VALUES (DEFAULT, ?, ?, ?, ?, ?, DEFAULT)',
			[fName, lName, role, email, hashedPassword],
		)
		if (inserted.affectedRows > 0) {	
			console.log(inserted.affectedRows)
			return res.status(201).send("Registered user successfully");
		} else {
			return res.status(500).send("Unexpected server error. User not registered");
		}
	}
	catch (err) {
		console.error('(/register Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});
server.post('/application', async (req: Request, res: Response) => {
	console.log('CREATE APPLICATION REQUEST')
	console.log(req.body);
	try {
		// Check if body contains all required properties
		const { fName, lName, email, dateOfBirth } = req.body;
		console.debug(fName, lName, email, dateOfBirth);
		if ( !fName || !lName  || !email || !dateOfBirth ) {
			return res.status(400).send('Malformed application request');
		}

		// Check if user already exists in external_users
		console.log(email)
		console.debug("before check")
		let [usersRows]: [mysql.RowDataPacket[], mysql.FieldPacket[]] = await dbPool.execute(
			'SELECT * FROM `external_users` WHERE `email` = ?',
			[email]
		);
		console.debug("after check")
		let userId: number;

		// Create user in external_users if they don't exist
		if (usersRows.length === 0) {
			const password = lName+fName;
			const hashedPassword = await hashString(password);
			console.debug()
			const [inserted] = await dbPool.execute<ResultSetHeader>(
				'INSERT INTO `external_users` VALUES (DEFAULT, ?, ?, ?, ?, ?, DEFAULT)',
				[fName, lName, dateOfBirth, email, hashedPassword]
			);
			if (inserted.affectedRows === 0) {
				return res.status(500).send('Failed to create user');
			}
			userId = inserted.insertId;
			res.write(`Created new user with password: ${password}\n`);
		} else {
			userId = usersRows[0].id;
		}

		// Check if user already has a pending application
		const [applicationsRows]: [mysql.RowDataPacket[], mysql.FieldPacket[]] = await dbPool.execute(
			'SELECT * FROM `applications` WHERE `user_id` = ? AND `status` = ?',
			[userId, 'pending']
		);
		if (applicationsRows.length > 0) {
			return res.status(409).send('User already has a pending application');
		}

		// Add new user to database
		const [inserted] = await dbPool.execute<ResultSetHeader>(
			'INSERT INTO `applications` VALUES (DEFAULT, ?, "Pending", DEFAULT)',
			[userId],
		)

		if (inserted.affectedRows > 0) {	
			console.log(inserted.affectedRows)
			res.status(201).write("Submitted application successfully");
			return res.end();
		} else {
			return res.status(500).send("Unexpected server error. User not registered");
		}
	}
	catch (err) {
		console.error('(POST /application Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});
server.get('/application', async (req: Request, res: Response) => {
	console.log('GET APPLICATIONS REQUEST')
	console.log(req.body);
	try {
		//TODO use where to implement filter and sort functions
		// Query to select all applications
		const [applicationsRows]: [mysql.RowDataPacket[], mysql.FieldPacket[]] = await dbPool.execute(
			[
				'SELECT external_users.f_name, external_users.l_name, applications.application_id,  applications.status, applications.submission_date',
				'FROM `applications`',
				'INNER JOIN external_users ON applications.user_id=external_users.user_id;'
			].join('\n')
		);
		if (applicationsRows.length < 0) {
			console.debug('no applications found')
			return res.status(204).send('No applications found');
		}
		
		const responseJSON = {
			"data": applicationsRows
		};

		console.debug('sending applications json')
		
		res.setHeader('Content-Type', 'application/json');
		return res.status(200).json(responseJSON);
	}
	catch (err) {
		console.error('(POST /application Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});

// Start the server
server.listen(port, () => {
	console.log(`Server listening at http://localhost:${port}`);
});

// Establish connection to MySQL database
const dbPool = await mysql.createPool({
	host: dbmsIP,
	user: 'bss',
	password: 'bss25',
	database: 'application_system',
	waitForConnections: true,
	connectionLimit: 50,
});

