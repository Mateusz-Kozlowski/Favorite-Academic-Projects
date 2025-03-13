package expressions;

import instructions.Block;

public class Modulo extends TwoArg {
	public Modulo(Expression left, Expression right) {
		super(left, right);
	}

	@Override
	public int evaluate(Block[] metaBlocks) throws Exception {
		return left.evaluate(metaBlocks) % right.evaluate(metaBlocks);
	}

	@Override
	public String toString() {
		return '(' + left.toString() + " % " + right.toString() + ')';
	}

	@Override
	public TwoArg deepCopy() { return new Modulo(left.deepCopy(), right.deepCopy()); }

	public static Modulo of(Expression left, Expression right) {
		return new Modulo(left, right);
	}
}
