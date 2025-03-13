package expressions;

import instructions.Block;

public class Literal extends OneArg{
	private int value;

	public Literal(int value) {
		this.value = value;
	}

	public Literal deepCopy() {
		return new Literal(value);
	}

	public void setValue(int value) { this.value = value; }

	public int evaluate(Block[] blocks) {
		return value;
	}

	public int evaluate() {
		return value;
	}

	@Override
	public String toString() {
		return String.valueOf(value);
	}

	public static Literal of(int value) {
		return new Literal(value);
	}
}
