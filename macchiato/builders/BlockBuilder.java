package builders;

import instructions.Assignment;
import instructions.Block;
import instructions.For;
import instructions.If;
import instructions.Instruction;
import instructions.Print;
import instructions.ProcedureCall;
import declarations.*;
import expressions.Expression;

import java.util.*;

public class BlockBuilder {
	private List<VariableDeclaration> variableDeclarations;

	private List<ProcedureDeclaration> procedureDeclarations;

	private List<Instruction> instructions;

	public BlockBuilder() {
		variableDeclarations = new ArrayList<>();
		procedureDeclarations = new ArrayList<>();
		instructions = new ArrayList<>();
	}

	public List<VariableDeclaration> getVariableDeclarations() {
		return variableDeclarations;
	}

	public List<ProcedureDeclaration> getProcedureDeclarations() {
		return procedureDeclarations;
	}

	public List<Instruction> getInstructions() {
		return instructions;
	}

	public BlockBuilder assign(char name, Expression expression) {
		instructions.add(new Assignment(name, expression));
		return this;
	}

	public BlockBuilder block(Block block) throws Exception {
		instructions.add(block);
		return this;
	}

	public BlockBuilder declareVariable(char name, Expression expression) {
		variableDeclarations.add(new VariableDeclaration(name, expression));
		return this;
	}

	public BlockBuilder declareVariables(VariableDeclaration[] variableDeclarations) {
		this.variableDeclarations.addAll(Arrays.asList(variableDeclarations));
		return this;
	}

	public BlockBuilder declareProcedure(String name, List<Character> argumentsNames, Block content) throws Exception {
		procedureDeclarations.add(new ProcedureDeclaration(name, argumentsNames, content));
		return this;
	}

	public BlockBuilder forLoop(For forLoop) {
		instructions.add(forLoop);
		return this;
	}

	public BlockBuilder ifElse(If ifElse) {
		instructions.add(ifElse);
		return this;
	}

	public BlockBuilder invoke(String name, List<Expression> argumentsExpressions) {
		instructions.add(new ProcedureCall(name, argumentsExpressions));
		return this;
	}

	public BlockBuilder print(Expression expression) {
		instructions.add(new Print(expression));
		return this;
	}

	public Block build() throws Exception {
		return new Block(this);
	}
}
