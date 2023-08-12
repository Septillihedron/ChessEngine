import { EngineAPI } from './engineAPI.mjs';
import { NdjsonStream } from './stream.mjs';

const endPoint =  "https://lichess.org/api";

const accessToken = "";

const updateDelay = 5000;

const headers = {
	Authorization: "Bearer " + accessToken
};

const listeners = {
	//challenge: acceptChallenge,
	gameStart: startEngine
};

async function fetchRetry(url, options, retries = 0, resolve = undefined) {
	if (retries >= 10) {
		resolve(undefined);
		return;
	}
	if (resolve) {
		fetch(url, options)
			.then(x => resolve(x))
			.catch(x => {
				if (retries >= 9) return
				console.log(`fetch failed ${retries} times, retrying...`);
				setTimeout(() => fetchRetry(url, options, retries + 1), 64*Math.pow(2, retries + 1));
			})
		return;
	}
	return new Promise(resolve => {
		fetchRetry(url, options, retries, resolve);
	})
}

(async function listenerHandler() {
	const url = `${endPoint}/stream/event`;
	new NdjsonStream(await fetchRetry(url, { headers }))
		.forEach(event => {
			console.log(event);
			let eventType = event.type;
			if (eventType in listeners) listeners[eventType](event);
		});
})();

function acceptChallenge(event) {
	let challengeId = event.challenge.id;
	let url = `${endPoint}/challenge/${challengeId}/accept`;
	fetch(url, { headers, method: "post" })
		.then(res => res.json())
		.then(status => status.ok? console.log(`challenge ${challengeId} accepted`) : undefined)
}

export async function delay(millis) {
	return new Promise(resolve => setTimeout(resolve, millis));
}

async function startEngine(event) {
	let game = event.game;
	let gameId = game.gameId;
	let fen = game.fen;

	let engine = new EngineAPI(game.color[0], game.fen);

	await delay(1000);

	let url = `${endPoint}/bot/game/stream/${gameId}`;
	let moveStream = new NdjsonStream(await fetchRetry(url, { headers }));

	await makeMove(gameId, await engine.playFirstMove())
	console.log("played first move")
	
	while (true) {
		let moveEvent = await moveStream.read();
		if (moveEvent == null) return;
		let bestMove;
		if (moveEvent.type == "gameState") {
			console.log(moveEvent.moves);
			bestMove = await engine.go(moveEvent.moves.split(" "));
		} else if (moveEvent.type == "gameFull") {
			//console.log(moveEvent.state.moves);
			//engine.startfen = moveEvent.initialFen;
			//bestMove = await engine.go(moveEvent.state.moves.split(" "));
		} else if (moveEvent.type == "gameFinish") {
			await fetch(`${endPoint}/challenge/ai`, { headers: {...headers, "Content-Type": "x-www-form-urlencoded"}, method: "POST", body: "level=6" });
			engine.close();
			break;
		}
		await makeMove(gameId, bestMove);
	}
}

async function makeMove(gameId, move, retries = 0) {
	if (retries >= 10) return;
	if (move === undefined) return;
	let url = `${endPoint}/bot/game/${gameId}/move/${move}`
	const res = await fetchRetry(url, { headers, method: "POST" })
	const status = await res.json();
	if (status.ok === true) {
		console.log(`bot: ${move}`);
	} else {
		console.log(`error: ${status.error}`);
		await new Promise(resolve => setTimeout(() => resolve, 64*Math.pow(2, retries + 1)))
		await makeMove(gameId, move, retries + 1)
	}
}
