package instructions;

import expressions.Literal;

import java.util.*;
import java.io.FileWriter;   // Import the FileWriter class

public abstract class Instruction {
	protected static int stepsCounter = 0;
	
	protected abstract void run(Block[] metaBlocks) throws Exception;
	
	protected void debuggerStop(Block[] biggerBlocks) throws Exception {
		Pair<Character, Literal> debugInput;

		do {
			Scanner scanner = new Scanner(System.in);

			System.out.print("Debugger input: ");
			String str = scanner.nextLine();

			debugInput = getDebuggerInput(str);

			if (debugInput.first == 'c') {
				debugInput.second.setValue(-1);
			}
			else if (debugInput.first == 'd') {
				printEvaluation(
					debugInput.second.evaluate(),
					biggerBlocks
				);
			}
			else if (debugInput.first == 'e') {
				System.exit(0);
			}
			else if (debugInput.first == 'm') {
				dump(str.substring(2), biggerBlocks);
			}
		} while (debugInput.first == 'd' || debugInput.second.evaluate() == 0);

		Instruction.stepsCounter = debugInput.second.evaluate();
	}
	
	static private void printEvaluation(int level, Block[] blocks) {
		if (level < 0 || level >= blocks.length) {
			System.out.println("Wrong nesting level, cannot print evaluation");
		}
		else {
			blocks[blocks.length - level - 1].printEvaluation(blocks);
		}
	}

	private Pair<Character, Literal> getDebuggerInput(String inputLine) {
		Pair<Character, Literal> res = new Pair<>('0', new Literal(0));

		if (inputLine.length() == 1 || inputLine.charAt(0) == 'm') {
			res.first = inputLine.charAt(0);
			res.second = new Literal(0);
			return res;
		}
		else {
			res.first = inputLine.charAt(0);
			int spacesCount = getSpacesCount(inputLine);

			try {
				res.second = new Literal(Integer.parseInt(inputLine.substring(1 + spacesCount)));
			}
			catch (Exception e) {
				System.out.println("Parsing the second argument (there is > 1 char in the input line) failed.");
				res.second = new Literal(0);
			}

			return res;
		}
	}

	static private int getSpacesCount(String s) {
		int res = 0;

		for (int i = 0; i< s.length(); i++) {
			if (s.charAt(i) == ' ') {
				res++;
			}
		}

		return res;
	}

	static private void dump(String path, Block[] blocks) throws Exception {
		System.out.println("Welcome to dump");

		FileWriter myWriter = new FileWriter(path);
		myWriter.write(blocks[blocks.length - 1].getMemoryDump(blocks));
		myWriter.close();

		System.out.println("Closed");
	}

	@Override
	public abstract String toString();

	public abstract Instruction deepCopy() throws Exception;
}
