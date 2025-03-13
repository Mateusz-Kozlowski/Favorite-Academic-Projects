package builders;

import expressions.Expression;
import instructions.If;
import instructions.Instruction;

import java.util.*;

public class IfElseBuilder {
	protected Expression left;

	protected String relation;

	protected Expression right;

	protected List<Instruction> then;

	protected List<Instruction> otherwise;

	public IfElseBuilder(Expression left, String relation, Expression right) {
		this.left = left;
		this.relation = relation;
		this.right = right;
		then = new ArrayList<>();
		otherwise = new ArrayList<>();
	}

	public Expression getLeft() {
		return left;
	}

	public String getRelation() {
		return relation;
	}

	public Expression getRight() {
		return right;
	}

	public List<Instruction> getThen() {
		return then;
	}

	public List<Instruction> getOtherwise() {
		return otherwise;
	}

	public IfElseBuilder otherwiseInstruction(Instruction instruction) {
		otherwise.add(instruction);
		return this;
	}

	public IfElseBuilder thenInstruction(Instruction instruction) {
		then.add(instruction);
		return this;
	}

	public If build() {
		return new If(this);
	}
}
