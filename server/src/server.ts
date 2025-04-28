import argon2 from 'argon2';
import crypto from 'crypto';
import mysql, { QueryError, QueryResult, ResultSetHeader } from 'mysql2/promise';
import express, { Request, Response } from 'express';
import cors from 'cors';
import router from './routers/apiRouter'
import 'dotenv/config'

const server = express();
server.use(express.json());

// Delay requests
// server.use((req, res, next) => setTimeout(next, 2000))
// server.use(router)
server.use(cors())
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
		const { user_type, email, password } = req.body;
		if ( !user_type || !email || !password) {
			console.debug('Malformed login request')
			return res.status(400).send('Malformed login request');
		}

		if (user_type != "internal" && user_type != "external") {
			console.log(user_type);
			return res.status(401).send('Incorrect user type');
		}

		let rows: mysql.RowDataPacket[];
		// Check if record with email exists
		if (user_type == "internal") {
			[rows] = await dbPool.execute<mysql.RowDataPacket[]>(
				'SELECT * FROM `internal_users` WHERE `email` = ?',
				[email]
			);
		} else if (user_type == "external") {
			[rows] = await dbPool.execute<mysql.RowDataPacket[]>(
				'SELECT * FROM `external_users` WHERE `email` = ?',
				[email]
			);			
		}

		if (rows.length === 0) {
			console.debug('Incorrect username or password')
			return res.status(401).send('Incorrect username or password');
		}

		// Check if password is correct
		const passwordMatches = await verifyHash(rows[0].password, password);
		if (!passwordMatches) {
			return res.status(401).send('Incorrect username or password');
		}

		console.debug('Logged in successfully')
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
		const { user_type, fName, lName, role, email, password } = req.body;
		console.debug(user_type, fName, lName, role, email, password);
		if (!user_type || !fName || !lName || !email || !password || (user_type === "internal" && !role)) {
			return res.status(400).send('Malformed register request');
		}

		if (user_type != "internal" && user_type != "external") {
			console.log(user_type);
			return res.status(401).send('Incorrect user type');
		}

		let rows: mysql.RowDataPacket[];
		// Check if user already exists
		if (user_type == "internal") {
			[rows] = await dbPool.execute<mysql.RowDataPacket[]>(
				'SELECT * FROM `internal_users` WHERE `email` = ?',
				[email]
			);
		} else if (user_type == "external") {
			[rows] = await dbPool.execute<mysql.RowDataPacket[]>(
				'SELECT * FROM `external_users` WHERE `email` = ?',
				[email]
			);
		}

		if (rows.length > 0) {
			return res.status(409).send('User already exists');
		}

		// Hash password
		const hashedPassword = await hashString(password);

		// Add new user to database
		if (user_type == "internal") {
			const [inserted] = await dbPool.execute<ResultSetHeader>(
				'INSERT INTO `internal_users` VALUES (DEFAULT, ?, ?, ?, ?, ?, DEFAULT)',
				[fName, lName, role, email, hashedPassword],
			);
			if (inserted.affectedRows > 0) {
				console.log(inserted.affectedRows);
				return res.status(201).send("Registered internal user successfully");
			}
		} else if (user_type == "external") {
			const [inserted] = await dbPool.execute<ResultSetHeader>(
				'INSERT INTO `external_users` VALUES (DEFAULT, ?, ?, NULL, ?, ?, DEFAULT)',
				[fName, lName, email, hashedPassword],
			);
			if (inserted.affectedRows > 0) {
				console.log(inserted.affectedRows);
				return res.status(201).send("Registered external user successfully");
			}
		}

		return res.status(500).send("Unexpected server error. User not registered");
	}
	catch (err) {
		console.error('(/register Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});
server.post('/application', async (req: Request, res: Response) => {
	console.log('CREATE APPLICATION REQUEST');
	console.log(req.body);
	try {
		// Check if body contains all required properties
		const { fName, lName, email, dateOfBirth, document } = req.body;
		// console.debug(fName, lName, email, dateOfBirth, document);
		if (!fName || !lName || !email || !dateOfBirth || !document) {
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

		 // Convert base64 document to a blob
		const documentBuffer = Buffer.from(document, 'base64');

		// Add new application to the database
		const [insertedApplication] = await dbPool.execute<ResultSetHeader>(
			'INSERT INTO `applications` VALUES (DEFAULT, ?, "Pending", DEFAULT)',
			[userId],
		);

		if (insertedApplication.affectedRows > 0) {
			const applicationId = insertedApplication.insertId;

			// Insert document into the documents table
			const [insertedDocument] = await dbPool.execute<ResultSetHeader>(
				'INSERT INTO `documents` VALUES (DEFAULT, ?, ?)',
				[applicationId, documentBuffer]
			);

			if (insertedDocument.affectedRows > 0) {
				console.log('Document uploaded successfully');
				res.status(201).write('Submitted application and uploaded document successfully');
				return res.end();
			} else {
				return res.status(500).send('Failed to upload document');
			}
		} else {
			return res.status(500).send('Unexpected server error. Application not registered');
		}
	}
	catch (err) {
		console.error('(POST /application Endpoint)', err);
		res.status(500).send(`Unexpected server error. ${err}`);
	}
});
server.put('/application', async (req: Request, res: Response) => {
	console.log('UPDATE APPLICATION REQUEST');
	console.log(req.body);
	try {
		// Check if body contains all required properties
		const { application_id, status } = req.body;
		if (!application_id || !status) {
			return res.status(400).send('Malformed application request');
		}

		// Update the application status in the database
		const [updatedApplication] = await dbPool.execute<ResultSetHeader>(
			'UPDATE `applications` SET status = ? WHERE application_id = ?',
			[status, application_id]
		);

		if (updatedApplication.affectedRows > 0) {
			return res.status(200).send(`Successfully updated application status to ${status}`);
		} else {
			return res.status(404).send('Application not found or no changes made');
		}
	} catch (err) {
		console.error('(PUT /application Endpoint)', err);
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
				'SELECT applications.*, external_users.*, documents.document_data',
				'FROM `applications`',
				'INNER JOIN external_users ON applications.user_id=external_users.user_id',
				'INNER JOIN documents ON applications.application_id=documents.application_id',
			].join('\n')
		);
		if (applicationsRows.length === 0) {
			return res.status(204).send('No applications found');
		}

		// Convert document blobs to base64
		applicationsRows.forEach((row) => {
			if (row['document_data']) {
				row['document_data'] = row['document_data'].toString('base64');
			}
		});
		
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

