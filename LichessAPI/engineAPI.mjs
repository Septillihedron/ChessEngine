
import { spawn } from "node:child_process"
import { delay } from "./index.mjs";

export class EngineAPI {

	engineProcess;

	startfen = "startpos";
	moves = "";
	color = "w";
	firstMove;

	constructor(color, fen) {
		console.log("initiating engine");
		this.color = color;
		this.engineProcess = spawn("../x64/Release/ChessEngine.exe", [fen]);
		this.engineProcess.stdin.write(color);
		this.engineProcess.stdout.on("data", data => {
			new TextDecoder().decode(data)
				.split("\n")
				.filter(line => line != "")
				.map(line => "Engine: " + line)
				.forEach(line => console.log(line));
		});
		this.firstMove = new Promise(resolve => {
			const bestMoveFinder = data => {
				if (data.length == 0) return;
				let bestMove = this._getBestMove(data);
				if (bestMove) {
					resolve(bestMove);
					this.engineProcess.stdout.removeListener("data", bestMoveFinder);
					return;
				}
			};
			this.engineProcess.stdout.on("data", bestMoveFinder);
		})
		console.log("finished initiating engine");
	}
	
	close() {
		engineProcess.exit();
	}

	/**
	 * 
	 * @returns {Promise<string>}
	 */
	playFirstMove() {
		return this.firstMove
	}

	addMoves(...moves) {
		this.moves.push(...moves);
	}

	setMoves(...moves) {
		this.moves = moves;
	}

	/**
	 * @param {string[]} moves 
	 * @returns {Promise<string>}
	 */
	async go(moves) {
		if (this.startfen === undefined) return undefined;
		if (moves.length % 2 != (this.color == 'b'? 1 : 0)) return undefined;
		
		const response = moves[moves.length - 1].trim();
		if (response == "") return;
		this.moves = moves;

		//console.log(response);
		this.engineProcess.stdin.write(response + "\r\n");

		return new Promise(resolve => {
			const bestMoveFinder = data => {
				if (data.length == 0) return;
				let bestMove = this._getBestMove(data);
				if (bestMove) {
					resolve(bestMove);
					this.engineProcess.stdout.removeListener("data", bestMoveFinder);
					return;
				}
			};
			this.engineProcess.stdout.on("data", bestMoveFinder);
		})
	}

	_getBestMove(stdOutput) {
		let output = new TextDecoder().decode(stdOutput);
		for (let line of output.split("\n")) {
			if (line.trim().match(/Best move: ([a-h][0-8]){2}[nbrq]?/)) {
				return line.substring("Best move: ".length);
			}
		}
	}

}
