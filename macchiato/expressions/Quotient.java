package expressions;

import instructions.Block;

public class Quotient extends TwoArg {
	public Quotient(Expression left, Expression right) {
		super(left, right);
	}

	@Override
	public int evaluate(Block[] metaBlocks) throws Exception {
		return left.evaluate(metaBlocks) / right.evaluate(metaBlocks);
	}

	@Override
	public String toString() {
		return '(' + left.toString() + " / " + right.toString() + ')';
	}

	@Override
	public TwoArg deepCopy() { return new Quotient(left.deepCopy(), right.deepCopy()); }

	public static Quotient of(Expression left, Expression right) {
		return new Quotient(left, right);
	}
}
