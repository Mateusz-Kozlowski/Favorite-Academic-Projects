package expressions;

import instructions.Block;

public class Sum extends TwoArg {

	public Sum(Expression left, Expression right) {
		super(left, right);
	}

	public int evaluate(Block[] metaBlocks) throws Exception {
		int leftValue = left.evaluate(metaBlocks);
		int rightValue = right.evaluate(metaBlocks);

		return leftValue + rightValue;
	}

	@Override
	public String toString() {
		return '(' + left.toString() + " + " + right.toString() + ')';
	}

	@Override
	public TwoArg deepCopy() { return new Sum(left.deepCopy(), right.deepCopy()); }

	public static Sum of(Expression left, Expression right) {
		return new Sum(left, right);
	}
}
