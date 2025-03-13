package instructions;

import expressions.Expression;
import builders.IfElseBuilder;
import java.util.*;

public class If extends Instruction {
	private Expression left;

	private Expression right;

	private String relation;

	private List<Instruction> then;

	private List<Instruction> otherwise;

	public If(IfElseBuilder ifElseBuilder) {
		this(
			ifElseBuilder.getLeft(),
			ifElseBuilder.getRelation(),
			ifElseBuilder.getRight(),
			ifElseBuilder.getThen(),
			ifElseBuilder.getOtherwise()
		);
	}

	private If() {}

	private If(
		Expression left,
		String relation,
		Expression right,
		List<Instruction> then) {

		this(
			left,
			relation,
			right,
			then,
			new ArrayList<>()
		);
	}

	private If(
		Expression left,
		String relation,
		Expression right,
		List<Instruction> then,
		List<Instruction> otherwise) {

		this.left = left;
		this.right = right;
		this.relation = relation;
		this.then = then;
		this.otherwise = otherwise;
	}

	public void run(Block[] blocks) throws Exception {
		if (Instruction.stepsCounter == 0) {
			System.out.println("Next instruction: calculating expression of " + this);
			debuggerStop(blocks);
		}

		int leftValue = 0;
		int rightValue = 0;

		try {
			leftValue = left.evaluate(blocks);
			rightValue = right.evaluate(blocks);
		}
		catch (Exception e) {
			System.out.print("Failed to calculate the expression of if: ");
			System.out.print(this);
			System.out.println("; " + e.getMessage());
			System.out.println("All variables visible in the block containing the instruction:");
			for (int i = blocks.length - 1; i >= 0; i--) {
				blocks[i].printEvaluation(blocks);
			}
			throw new Exception("End of program");
		}

		boolean conditionMet = false;

		if (Objects.equals(relation, "=")) {
			if (leftValue == rightValue) {
				conditionMet = true;
			}
		}
		else if (Objects.equals(relation, "<>")) {
			if (leftValue != rightValue) {
				conditionMet = true;
			}
		}
		else if (Objects.equals(relation, "<")) {
			if (leftValue < rightValue) {
				conditionMet = true;
			}
		}
		else if (Objects.equals(relation, ">")) {
			if (leftValue > rightValue) {
				conditionMet = true;
			}
		}
		else if (Objects.equals(relation, "<=")) {
			if (leftValue <= rightValue) {
				conditionMet = true;
			}
		}
		else if (Objects.equals(relation, ">=")) {
			if (leftValue >= rightValue) {
				conditionMet = true;
			}
		}

		Instruction.stepsCounter--; // calculating if condition

		if (conditionMet) {
			for (Instruction instruction : then) {
				instruction.run(blocks);
			}
		}
		else {
			if (otherwise != null) {
				for (Instruction instruction : otherwise) {
					instruction.run(blocks);
				}
			}
		}
	}

	@Override
	public String toString() {
		return "if (" + left.toString() + ' ' + relation + ' ' + right.toString() + ')';
	}

	@Override
	public If deepCopy() throws Exception {
		If res = new If();

		res.left = left.deepCopy();
		res.right = right.deepCopy();
		res.relation = relation;

		res.then = new ArrayList<>();

		for (Instruction instruction : then) {
			res.then.add(instruction.deepCopy());
		}

		res.otherwise = new ArrayList<>();

		for (Instruction instruction : otherwise) {
			res.otherwise.add(instruction.deepCopy());
		}

		return res;
	}
}
