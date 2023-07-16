import { EngineAPI } from './engineAPI.mjs';
import { NdjsonStream } from './stream.mjs';

const endPoint =  "https://lichess.org/api";

const accessToken = "";

const updateDelay = 5000;

const headers = {
	Authorization: "Bearer " + accessToken
};

const listeners = {
	challenge: acceptChallenge,
	gameStart: startEngine
};

(async function listenerHandler() {
	const url = `${endPoint}/stream/event`;
	new NdjsonStream(await fetch(url, { headers }))
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

	let engine = new EngineAPI(game.color[0]);

	await delay(10000);

	let url = `${endPoint}/bot/game/stream/${gameId}`;
	let moveStream = new NdjsonStream(await fetch(url, { headers }));

	if (game.color[0] == 'w') await makeMove(gameId, await engine.playFirstMove())
	
	while (true) {
		let moveEvent = await moveStream.read();
		if (moveEvent == null) return;
		let bestMove;
		if (moveEvent.type == "gameState") {
			console.log(moveEvent.moves);
			bestMove = await engine.go(moveEvent.moves.split(" "));
		} else if (moveEvent.type == "gameFull") {
			console.log(moveEvent.state.moves);
			engine.startfen = moveEvent.initialFen;
			bestMove = await engine.go(moveEvent.state.moves.split(" "));
		}
		await makeMove(gameId, bestMove);
	}
}

async function makeMove(gameId, move) {
	if (move === undefined) return;
	let url = `${endPoint}/bot/game/${gameId}/move/${move}`
	fetch(url, { headers, method: "post" })
		.then(res => res.json())
		.then(status => {
			if (status.ok === true) {
				console.log(`bot: ${move}`);
			} else {
				console.log(`error: ${status.error}`);
			}
		})
}
