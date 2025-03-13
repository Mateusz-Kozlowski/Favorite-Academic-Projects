package declarations;

import expressions.Expression;
import expressions.Literal;
import instructions.Block;

public class VariableDeclaration {
	private final char name;

	private Expression expression;

	protected Boolean initialized = false;

	public boolean isInitialized() {
		return initialized;
	}

	public void setInitialized(boolean initialized) { this.initialized = initialized; }

	public VariableDeclaration(char name, Expression expression) {
		this.name = name;
		this.expression = expression;
	}

	public VariableDeclaration deepCopy() {
		VariableDeclaration res = new VariableDeclaration(name, expression.deepCopy());
		res.setInitialized(initialized);
		return res;
	}

	public void setValue(int value) {
		expression = new Literal(value);
	}

	public int getExpressionValue(Block[] blocks) throws Exception {
		return expression.evaluate(blocks);
	}

	public char getName() {
		return name;
	}

	public Expression getExpression() {
		return expression;
	}

	public String toString() {
		return name + " = " + expression.toString();
	}
}
