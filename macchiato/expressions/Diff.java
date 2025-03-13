package expressions;

import instructions.Block;

public class Diff extends TwoArg {
	public Diff(Expression left, Expression right) {
		super(left, right);
	}

	@Override
	public int evaluate(Block[] metaBlocks) throws Exception {
		return left.evaluate(metaBlocks) - right.evaluate(metaBlocks);
	}

	@Override
	public String toString() {
		return '(' + left.toString() + " - " + right.toString() + ')';
	}

	@Override
	public TwoArg deepCopy() { return new Diff(left.deepCopy(), right.deepCopy()); }

	public static Diff of(Expression left, Expression right) {
		return new Diff(left, right);
	}
}
