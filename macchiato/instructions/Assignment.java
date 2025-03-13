package instructions;

import expressions.Expression;

public class Assignment extends Instruction {
	private char name;
	private Expression expression;

	private Assignment() {}

	public Assignment(char name, Expression expression) {
		this.name = name;
		this.expression = expression;
	}

	public void run(Block[] blocks) throws Exception {
		if (Instruction.stepsCounter == 0) {
			System.out.print("Next instruction: " + this);
			debuggerStop(blocks);
		}

		for (int i = blocks.length - 1; i >= 0; i--) {
			if (blocks[i].containsInitializedVariable(name)) {
				try {
					blocks[i].assign(name, expression.evaluate(blocks));
				}
				catch (Exception e) {
					System.out.print("Failed to calculate the expression of assignment: ");
					System.out.print(this);
					System.out.println("; " + e.getMessage());
					System.out.println("All variables visible in the block containing the instruction:");
					for (int j = blocks.length - 1; j >= 0; j--) {
						blocks[j].printEvaluation(blocks);
					}
					throw new Exception("End of program");
				}

				Instruction.stepsCounter--; // assignment

				return;
			}
		}

		System.out.print("Cannot assign to " + name + ", the variable wasn't found");
		System.out.println("All variables visible in the block containing the instruction:");
		for (int j = blocks.length - 1; j >= 0; j--) {
			blocks[j].printEvaluation(blocks);
		}
		throw new Exception("End of program");
	}

	@Override
	public String toString() {
		return "assignment " + name + " = " + expression.toString() + '\n';
	}

	@Override
	public Assignment deepCopy() throws Exception {
		Assignment res = new Assignment();

		res.name = name;
		res.expression = expression.deepCopy();

		return res;
	}
}
