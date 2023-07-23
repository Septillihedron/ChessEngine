
import java.util.List;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.HashSet;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.IOException;

class PerftTester {
	
	StockfishInterface sf;
	MyEngineInterface me;
	
	static final String[] testFens = {
		"startpos",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", // to depth 5
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", // to depth 6
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", // to depth 5
		"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -", // to depth 5
		"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -", // to depth 5
	};

	static final List<String> moves = new ArrayList<>(10);

	public static void main(String[] args) throws IOException {
		System.out.println("Hello world!");
		new PerftTester();
	}
	
	PerftTester() throws IOException {
		sf = new StockfishInterface(Runtime.getRuntime().exec("fish.exe"));
		System.out.println();
		out:
		for (int i=0; i<testFens.length; i++) {
			int maxDepth = i == 2? 6 : 5;
			String fen = testFens[i];
			System.out.println("fen: " + fen);
			for (int depth=1; depth<=maxDepth; depth++) {
				if (!test(fen, depth)) break out;
				System.out.println("depth: " + depth);
			}
			System.out.println();
		}
		sf.stockfish.destroy();
	}
	
	boolean test(String fen, int depth) throws IOException {
		sf.fen = fen;
		sf.maxDepth = depth;
		me = new MyEngineInterface(Runtime.getRuntime().exec(new String[]{"x64/Release/ChessEngine.exe", fen, depth+""}));
		boolean success = test();
		me.myEngine.destroy();
		return success;
	}
	
	boolean test() throws IOException {
		var sfResult = sf.perft(moves);
		var meResults = me.read();
		var moveIndexes = meResults.getKey();
		var meResult = meResults.getValue();
		if (sfResult.get("total").longValue() == meResult.get("total").longValue()) return true;
		var allMoves = new HashSet<>(sfResult.keySet());
		allMoves.addAll(meResult.keySet());
		for (String move : allMoves) {
			if (move.equals("total")) continue;
			Long sfValue = sfResult.get(move);
			Long meValue = meResult.get(move);
			if (sfValue == null || meValue == null) {
				System.out.println("  ".repeat(moves.size()) + move + " diff " + ((meValue == null)? "me" : "sf") + " null");
				return false;
			}
			if (sfResult.get(move).longValue() == meResult.get(move).longValue()) continue;
			System.out.println("  ".repeat(moves.size()) + move);
			
			moves.add(move);
			me.send(moveIndexes.get(move));
			
			test();
			
			me.send(-2);
			me.read();
			moves.remove(moves.size()-1);
			return false;
		}
		return true;
	}

	static class StockfishInterface {
		Process stockfish;
		BufferedReader output;
		BufferedWriter input;
		String fen;
		int maxDepth;
		
		StockfishInterface(Process stockfish) {
			System.out.println("SF interface initiating");
			this.stockfish = stockfish;
			input = stockfish.outputWriter();
			output = stockfish.inputReader();
			while (true) {
				try {
					String line = output.readLine();
					if (line != null) break;
				} catch (IOException e) {}
			}
			System.out.println("SF interface initiated");
		}

		Map<String, Long> perft(List<String> moves) throws IOException {
			String setPositionCommand = String.format("position %s moves %s", fen.equals("startpos")? fen : "fen " + fen, String.join(" ", moves));
			String perftCommand = String.format("go perft %d", maxDepth - moves.size());
			if (maxDepth - moves.size() == 0) return Map.of("total", 1L);
			
			input.write(setPositionCommand, 0, setPositionCommand.length());
			input.newLine();
			input.write(perftCommand, 0, perftCommand.length());
			input.newLine();
			input.flush();
			
			Map<String, Long> perftResult = new HashMap<>(100);
			
			while (true) {
				String line = output.readLine();
				if (line == null) continue;
				if (line.isEmpty()) continue;
				String[] keyval = line.split(": ");
				if (keyval.length != 2) continue;
				if (keyval[0].equals("Nodes searched")) keyval[0] = "total";
				perftResult.put(keyval[0], Long.valueOf(keyval[1]));
				if (keyval[0].equals("total")) break;
			}
			
			return perftResult;
		}
	}
	
	static class MyEngineInterface {
		Process myEngine;
		BufferedReader output;
		BufferedWriter input;
		
		MyEngineInterface(Process myEngine) {
			this.myEngine = myEngine;
			input = myEngine.outputWriter();
			output = myEngine.inputReader();
		}
		
		void send(int index) throws IOException {
			String indexStr = index + "";
			input.write(indexStr, 0, indexStr.length());
			input.newLine();
			input.flush();
		}
		
		Map.Entry<Map<String, Integer>, Map<String, Long>> read() throws IOException {
			Map<String, Long> perftResult = new HashMap<>(100);
			Map<String, Integer> moveIndexes = new HashMap<>(100);
			
			while (true) {
				String line = output.readLine();
				if (line == null) continue;
				if (line.isEmpty()) continue;
				String[] keyval = line.split(": ");
				if (keyval.length <= 1) continue;
				if (keyval[0].equals("Nodes searched")) {
					perftResult.put("total", Long.valueOf(keyval[1]));
					break;
				}
				perftResult.put(keyval[1], Long.valueOf(keyval[2]));
				moveIndexes.put(keyval[1], Integer.valueOf(keyval[0].strip()));
			}
			
			return Map.entry(moveIndexes, perftResult);
		}
	}

}