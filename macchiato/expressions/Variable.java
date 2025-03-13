package expressions;

import instructions.Block;

public class Variable extends Expression {
	private final char name;

	public Variable(char name) {
		this.name = name;
	}

	public Variable deepCopy() {
		return new Variable(name);
	}

	public int evaluate(Block[] blocks) throws Exception {
		for (int i = blocks.length - 1; i >= 0; i--) {
			if (blocks[i].containsInitializedVariable(name)) {
				return blocks[i].getVariableValue(name, blocks);
			}
		}

		throw new Exception("There is no initialized variable named " + name);
	}

	@Override
	public String toString() {
		return String.valueOf(name);
	}

	public static Variable named(char name) {
		return new Variable(name);
	}
}
